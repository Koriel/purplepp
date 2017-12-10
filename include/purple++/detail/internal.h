//
// Created by int3 on 06.12.17.
//

#pragma once

// TODO: fix include
//#include <glib.h>
#include <glib-2.0/glib.h>
#include <purple++/detail/util.h>
#include <boost/iterator_adaptors.hpp>

namespace purplepp {

namespace detail {

template <class ValuePtr>
class glist {
	static_assert(std::is_pointer<ValuePtr>::value, "ValuePtr must be pointer to some real data");

	GList* _list;

	class iterator : public boost::iterator_facade<iterator, ValuePtr, boost::forward_traversal_tag> {
		friend class boost::iterator_core_access;

		GList* _list;
	public:
		iterator(GList* list) : _list(list) {}

		bool equal(const iterator& rhs) const {
			return _list == rhs._list;
		}

		void increment() {
			_list = g_list_next(_list);
		}

		ValuePtr& dereference() const {
			return reinterpret_cast<ValuePtr&>(_list->data);
		}
	};
public:
	glist(GList* list) : _list(list) {}

	iterator begin() { return _list; }
	iterator end() { return nullptr; }

	size_t size() const { return g_list_length(_list); }
};

template <class Key, class Value>
class ghashtable {
	using result_t = std::pair<Key, Value>;

	GHashTable* _table;

	class iterator : public boost::iterator_facade<iterator, result_t, boost::forward_traversal_tag, result_t> {
		friend class boost::iterator_core_access;

		GHashTableIter it;
		bool end;
	public:
		iterator(GHashTable* table) {
			if (table) {
				g_hash_table_iter_init(&it, table);
				end = false;
			}
			else {
				end = true;
			}
		}

		bool equal(const iterator& rhs) const {
			return end == rhs.end;
		}

		void increment() {
			if (end) return;

			void* temp1;
			void* temp2;
			end = g_hash_table_iter_next(&it, &temp1, &temp2) == FALSE;
		}

		result_t dereference() const {
			GHashTableIter temp{};
			std::memcpy(&temp, &it, sizeof(it));

			void* key;
			void* value;
			g_hash_table_iter_next(&temp, &key, &value);
			return std::make_pair(reinterpret_cast<Key>(key), reinterpret_cast<Value>(value));
		}
	};
public:
	ghashtable(GHashTable* table) : _table(table) {}

	iterator begin() {
		return iterator{ _table };
	}

	iterator end() {
		return iterator{ nullptr };
	}
};

} // namespace detail

} // namespace purplepp