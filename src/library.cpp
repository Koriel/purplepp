//
// Created by int3 on 25.11.17.
//

#include <purple++/library.h>

#include <purple++/core/saved_status.h>
#include <purple++/core/account.h>
#include <purple++/core/connection.h>
#include <purple++/core/conversation.h>
#include <purple++/core/blist.h>
#include <purple++/detail/util.h>
#include <fmt/printf.h>

#include <libpurple/purple.h>

#include <vector>
#include <mutex>
#include <list>
#include <thread>
#include <unordered_map>

namespace purplepp {

static const char UI_ID[] = "purple++-daemon";

static void ui_init();

static PurpleCoreUiOps core_uiops {
		nullptr, /* ui_prefs_init */
		nullptr, /* debug_ui_init */
		ui_init, /* ui_init */
		nullptr, /* quit */

		/* padding */
		nullptr, /* get_ui_info */
		nullptr, /* reserved1 */
		nullptr, /* reserved2 */
		nullptr  /* reserved3 */
};

/**
 * The following eventloop functions are used in both pidgin and purple-text. If your
 * application uses glib mainloop, you can safely use this verbatim.
 */
#define PURPLE_GLIB_READ_COND  (G_IO_IN  | G_IO_HUP | G_IO_ERR)
#define PURPLE_GLIB_WRITE_COND (G_IO_OUT | G_IO_HUP | G_IO_ERR | G_IO_NVAL)

typedef struct _PurpleGLibIOClosure {
	PurpleInputFunction function;
	guint result;
	gpointer data;
} PurpleGLibIOClosure;

static void purple_glib_io_destroy(gpointer data)
{
	g_free(data);
}

static gboolean purple_glib_io_invoke(GIOChannel *source, GIOCondition condition, gpointer data)
{
	PurpleGLibIOClosure *closure = static_cast<PurpleGLibIOClosure*>(data);
	std::underlying_type<PurpleInputCondition>::type purple_cond = 0;

	if (condition & PURPLE_GLIB_READ_COND)
		purple_cond |= PURPLE_INPUT_READ;
	if (condition & PURPLE_GLIB_WRITE_COND)
		purple_cond |= PURPLE_INPUT_WRITE;

	closure->function(closure->data, g_io_channel_unix_get_fd(source),
					  static_cast<PurpleInputCondition>(purple_cond));

	return TRUE;
}

static guint glib_input_add(gint fd, PurpleInputCondition condition, PurpleInputFunction function,
							gpointer data)
{
	PurpleGLibIOClosure *closure = g_new0(PurpleGLibIOClosure, 1);
	GIOChannel *channel;
	std::underlying_type<GIOCondition>::type cond = 0;

	closure->function = function;
	closure->data = data;

	if (condition & PURPLE_INPUT_READ)
		cond |= PURPLE_GLIB_READ_COND;
	if (condition & PURPLE_INPUT_WRITE)
		cond |= PURPLE_GLIB_WRITE_COND;

	channel = g_io_channel_unix_new(fd);
	closure->result = g_io_add_watch_full(channel, G_PRIORITY_DEFAULT, static_cast<GIOCondition>(cond),
										  purple_glib_io_invoke, closure, purple_glib_io_destroy);

	g_io_channel_unref(channel);
	return closure->result;
}

static PurpleEventLoopUiOps glib_eventloops =
		{
				g_timeout_add,
				g_source_remove,
				glib_input_add,
				g_source_remove,
				nullptr,
#if GLIB_CHECK_VERSION(2,14,0)
				g_timeout_add_seconds,
#else
				nullptr,
#endif
				/* padding */
				nullptr,
				nullptr,
				nullptr
		};

struct library_impl {
	enum class library_state {
		none,
		loop_run,
		loop_requested_stop,
		loop_stopped
	};

	// TODO: setters for all this params in purplepp::library class
	std::string custom_user_directory = ".purple";
	bool debug_enabled = false;
	std::string custom_plugin_path = "";
	std::string plugin_save_pref = "/purple/purple-daemon/plugins/saved";
	std::unordered_map<std::string, std::unique_ptr<account>> accounts;
	std::thread::id loop_thread_id;
	GMainLoop *loop = nullptr;
	volatile library_state state = library_state::none;

	std::list<std::function<void()>> tasks;
	std::mutex tasks_mutex;

	void init() {
		/* Set a custom user directory (optional) */
		if (!custom_user_directory.empty()) {
			purple_util_set_user_dir(custom_user_directory.c_str());
		}

		/* We do not want any debugging for now to keep the noise to a minimum. */
		purple_debug_set_enabled(debug_enabled ? TRUE : FALSE);

		/* Set the core-uiops, which is used to
		 *      - initialize the ui specific preferences.
		 *      - initialize the debug ui.
		 *      - initialize the ui components for all the modules.
		 *      - uninitialize the ui components for all the modules when the core terminates.
		 */
		purple_core_set_ui_ops(&core_uiops);

		/* Set the uiops for the eventloop. If your client is glib-based, you can safely
		 * copy this verbatim. */
		purple_eventloop_set_ui_ops(&glib_eventloops);

		/* Set path to search for plugins. The core (libpurple) takes care of loading the
		 * core-plugins, which includes the protocol-plugins. So it is not essential to add
		 * any path here, but it might be desired, especially for ui-specific plugins. */
		purple_plugins_add_search_path(custom_plugin_path.c_str());

		/* Now that all the essential stuff has been set, let's try to init the core. It's
		 * necessary to provide a non-NULL name for the current ui to the core. This name
		 * is used by stuff that depends on this ui, for example the ui-specific plugins. */
		if (purple_core_init(UI_ID) == 0) {
			/* Initializing the core failed. Terminate. */
			fmt::fprintf(stderr, "libpurple initialization failed. Dumping core.\n");
			fmt::fprintf(stderr, "Please report this!\n");
			abort();
		}

		/* Create and load the buddylist. */
		purple_set_blist(purple_blist_new());
		purple_blist_load();

		/* Load the preferences. */
		//purple_prefs_load();

		/* Load the desired plugins. The client should save the list of loaded plugins in
		 * the preferences using purple_plugins_save_loaded(plugin_save_pref) */
		purple_plugins_load_saved(plugin_save_pref.c_str());

		/* Load the pounces. */
		purple_pounces_load();
	}

	void run_loop() {
		loop = g_main_loop_new(nullptr, FALSE);

		for (auto& acc : accounts) {
			acc.second->connect();
		}

		fmt::print(stderr, "Loop ran in thread {}\n", std::this_thread::get_id());

		loop_thread_id = std::this_thread::get_id();
		state = library_state::loop_run;
		g_main_loop_run(loop);

		// after getting stop_loop
		//purple_timeout_remove(loop_timer);
		g_main_loop_unref(loop);
		loop = nullptr;
		state = library_state::loop_stopped;
	}

	void stop_loop() {
		state = library_state::loop_requested_stop;
	}

	void wait_loop() {
		while (state != library_state::loop_stopped) {
			std::this_thread::sleep_for(std::chrono::milliseconds{ 1 });
		}
	}

	bool on_tick() {
		// copy all tasks
		decltype(tasks) copy_tasks;
		{
			std::lock_guard<std::mutex> _{ tasks_mutex };
			if (!tasks.empty()) {
				copy_tasks = std::move(tasks);
			}
		}

		for (auto& task : copy_tasks) {
			task();
		}

		if (state == library_state::loop_requested_stop) {
			fmt::print(stderr, "Calling g_main_loop_quit..\n");
			g_main_loop_quit(loop);
			return false;
		}

		return true;
	}

	struct PurpleBuddy;
	static void buddy_status_changed(PurpleBuddy *buddy, PurpleStatus *old_status, PurpleStatus *status) {
		fmt::print("buddy_status_changed: {}, {}", (void*)old_status, (void*)status);
	}

	void init_signals() {
		static int handle;

		purple_signal_connect(purple_connections_get_handle(), "signed-on", &handle, reinterpret_cast<PurpleCallback>(signed_on), nullptr);

		//purple_signal_connect(purple_blist_get_handle(), "buddy-status-changed", &handle, reinterpret_cast<PurpleCallback>(buddy_status_changed), nullptr);

		//purple_signal_register(purple_connections_get_handle(), "new-task", purple_marshal_VOID__POINTER, nullptr, 1, purple_value_new(PURPLE_TYPE_POINTER));
		//purple_signal_connect(purple_connections_get_handle(), "new-task", &handle, PURPLE_CALLBACK(new_task_handler), nullptr);

		purple_timeout_add(1, [](gpointer impl){
			return ((library_impl*)impl)->on_tick() ? 1 : 0;
		}, this);
	}

	void add_task(std::function<void()> task) {
		if (std::this_thread::get_id() == loop_thread_id) {
			fmt::print(stderr, "Executing task immediately.. {}\n", std::this_thread::get_id());
			task();
		}
		else {
			fmt::print(stderr, "Executing task delayed.. {}\n", std::this_thread::get_id());

			std::lock_guard<std::mutex> _{ tasks_mutex };
			tasks.push_back(std::move(task));
			fmt::print("Tasks count: {}\n", tasks.size());
			//purple_signal_emit(purple_connections_get_handle(), "new-task", arg);
		}
	}

	account* find_account(boost::string_view login) {
		auto it = accounts.find(login.to_string());
		return it == accounts.end() ? nullptr : it->second.get();
	}

	static void signed_on(PurpleConnection *gc)
	{
		auto& conn = connection::_get_wrapper(gc);
		conn._trigger_signed_on();
	}
};

library_impl& get_library_impl() {
	static library_impl impl;
	return impl;
}

void library::init() {
	auto& impl = get_library_impl();

	impl.init();
	impl.init_signals();

	//purplepp::saved_status status("Online", status_primitive::available);
	//status.activate();
}

void library::add_account(std::unique_ptr<account> account) {
	auto& impl = get_library_impl();

	account->set_enabled(UI_ID, true);
	auto username = account->get_username().to_string();
	impl.accounts[std::move(username)] = std::move(account);
}

void library::run_loop() {
	get_library_impl().run_loop();
}

void library::stop_loop() {
	get_library_impl().stop_loop();
}

void library::wait_loop() {
	get_library_impl().wait_loop();
}

void library::add_task(std::function<void()> task) {
	get_library_impl().add_task(std::move(task));
}

void library::set_debug(bool enabled) {
	get_library_impl().debug_enabled = enabled;
}

account* library::find_account(boost::string_view login) {
	return get_library_impl().find_account(login);
}

std::thread::id library::get_loop_thread_id() {
	return get_library_impl().loop_thread_id;
}

static void ui_init()
{
	/**
	 * This should initialize the UI components for all the modules.
	 */

	static PurpleConnectionUiOps connection_uiops {
			[](PurpleConnection *gc, const char *text, size_t step, size_t step_count) {
				connection::_get_wrapper(gc).on_connect_progress(text, step, step_count);
			},
			[](PurpleConnection *gc) {
				connection::_get_wrapper(gc).on_connected();
			},
			[](PurpleConnection *gc) {
				connection::_get_wrapper(gc).on_disconnected();
			},
			[](PurpleConnection *gc, const char *text){
				connection::_get_wrapper(gc).on_notice(text);
			},
			[](PurpleConnection *gc, const char *text) {
				connection::_get_wrapper(gc).on_report_disconnect(text);
			},
			[](){
				fmt::print("network_connected\n");
			},
			[](){
				fmt::print("network_disconnected\n");
			},
			[](PurpleConnection *gc, PurpleConnectionError reason, const char *text) {
				connection::_get_wrapper(gc).on_report_disconnect_reason((int)reason, text);
			},
			nullptr,
			nullptr,
			nullptr
	};
	purple_connections_set_ui_ops(&connection_uiops);

	static PurpleAccountUiOps account_uiops {
			[](PurpleAccount *account, const char *remote_user, const char *id, const char *alias, const char *message) {
				account::_get_wrapper(account)->notify_added(remote_user, id, alias, message);
			},
			[](PurpleAccount *account, PurpleStatus *status) {
				// BugFix:
				//  this callback can be called when account is being created
				//  so we should check, that wrapper is already set
				auto wrapper = account::_get_wrapper(account);
				if (wrapper) {
					wrapper->status_changed(status);
				}
			},
			[](PurpleAccount *account, const char *remote_user, const char *id, const char *alias, const char *message){
				account::_get_wrapper(account)->request_add(remote_user, id, alias, message);
			},
			[](PurpleAccount *account, const char *remote_user, const char *id, const char *alias, const char *message,
			   gboolean on_list, PurpleAccountRequestAuthorizationCb authorize_cb, PurpleAccountRequestAuthorizationCb deny_cb, void *user_data) {
				return account::_get_wrapper(account)->request_authorize(remote_user, id, alias, message, on_list != 0, user_data);
			},
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr
	};
	purple_accounts_set_ui_ops(&account_uiops);

	static PurpleConversationUiOps conv_uiops {
			[](PurpleConversation *conv) { // create
				//account::_get_wrapper(conv->account)->_create_conversation(conv);
			},
			[](PurpleConversation *conv) { // destroy
				//account::_get_wrapper(conv->account)->_destroy_conversation(conv);
			},
			nullptr,                      /* write_chat           */
			nullptr,                      /* write_im             */
			[](PurpleConversation *conv, const char *name, const char *alias, const char *message, PurpleMessageFlags flags, time_t mtime) { // write_conv
				account::_create_conversation(conv)->write_conv(name ? name : "", alias ? alias : "", message, flags, mtime);
			},
			[](PurpleConversation *conv, GList *cbuddies, gboolean new_arrivals) -> void {

			},                      	  /* chat_add_users       */
			nullptr,                      /* chat_rename_user     */
			nullptr,                      /* chat_remove_users    */
			nullptr,                      /* chat_update_user     */
			nullptr,                      /* present              */
			nullptr,                      /* has_focus            */
			nullptr,                      /* custom_smiley_add    */
			nullptr,                      /* custom_smiley_write  */
			nullptr,                      /* custom_smiley_close  */
			nullptr,                      /* send_confirm         */
			nullptr,                      /* reserved1         */
			nullptr,                      /* reserved2         */
			nullptr,                      /* reserved3         */
			nullptr                       /* reserved4         */
	};
	purple_conversations_set_ui_ops(&conv_uiops);

	static PurpleBlistUiOps blist_uiops = {
			nullptr, // void (*new_list)(PurpleBuddyList *list); /**< Sets UI-specific data on a buddy list. */

			[](PurpleBlistNode *_node){
				blist_node node{ _node };

				if (node.get_type() == blist_node_type::buddy) {
					fmt::print("[new_node] {}\n", node);

					// temporary fix (kostyl')
					auto buddy = node.as_buddy();

					PurpleConversation *imconv = purple_conversation_new(
							static_cast<PurpleConversationType>(conversation_type::im),
							buddy.get_account()->_get_impl(),
							buddy.get_name().to_string().c_str()
					);
					purple_conversation_present(imconv);
				}
			}, // void (*new_node)(PurpleBlistNode *node); /**< Sets UI-specific data on a node. */

			[](PurpleBuddyList *list){
				fmt::print("show blist: {}\n", (void*)list);
			}, // void (*show)(PurpleBuddyList *list);     /**< The core will call this when it's finished doing its core stuff */

			nullptr, // void (*update)(PurpleBuddyList *list, PurpleBlistNode *node);       /**< This will update a node in the buddy list. */

			nullptr, // void (*remove)(PurpleBuddyList *list, PurpleBlistNode *node);       /**< This removes a node from the list */

			nullptr, // void (*destroy)(PurpleBuddyList *list);  /**< When the list is destroyed, this is called to destroy the UI. */

			nullptr, // void (*set_visible)(PurpleBuddyList *list, gboolean show);            /**< Hides or unhides the buddy list */

			[](PurpleAccount *account, const char *username, const char *group, const char *alias){
				fmt::print("account: {}, username: {}, group: {}, alias: {}\n", account::_get_wrapper(account)->get_username(), username, group, alias);
			}, // void (*request_add_buddy)(PurpleAccount *account, const char *username, const char *group, const char *alias);

			[](PurpleAccount *account, PurpleGroup *group, const char *alias, const char *name){
				fmt::print("account: {}, group: {}, alias: {}, name: {}\n", account::_get_wrapper(account)->get_username(), (void*)group, alias, name);
			}, // void (*request_add_chat)(PurpleAccount *account, PurpleGroup *group, const char *alias, const char *name);

			nullptr, // void (*request_add_group)(void);

			/**
			 * This is called when a node has been modified and should be saved.
			 *
			 * Implementation of this UI op is OPTIONAL. If not implemented, it will
			 * be set to a fallback function that saves data to blist.xml like in
			 * previous libpurple versions.
			 *
			 * @param node    The node which has been modified.
			 *
			 * @since 2.6.0.
			 */
			nullptr, // void (*save_node)(PurpleBlistNode *node);

			/**
			 * Called when a node is about to be removed from the buddy list.
			 * The UI op should update the relevant data structures to remove this
			 * node (for example, removing a buddy from the group this node is in).
			 *
			 * Implementation of this UI op is OPTIONAL. If not implemented, it will
			 * be set to a fallback function that saves data to blist.xml like in
			 * previous libpurple versions.
			 *
			 * @param node  The node which has been modified.
			 * @since 2.6.0.
			 */
			nullptr, // void (*remove_node)(PurpleBlistNode *node);

			/**
			 * Called to save all the data for an account. If the UI sets this,
			 * the callback must save the privacy and buddy list data for an account.
			 * If the account is NULL, save the data for all accounts.
			 *
			 * Implementation of this UI op is OPTIONAL. If not implemented, it will
			 * be set to a fallback function that saves data to blist.xml like in
			 * previous libpurple versions.
			 *
			 * @param account  The account whose data to save. If NULL, save all data
			 *                  for all accounts.
			 * @since 2.6.0.
			 */
			nullptr, // void (*save_account)(PurpleAccount *account);

			nullptr // void (*_purple_reserved1)(void);
	};
	purple_blist_set_ui_ops(&blist_uiops);
}

} // namespace purplepp