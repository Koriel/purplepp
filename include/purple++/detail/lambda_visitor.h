//
// Created by int3 on 09.12.17.
//

#pragma once

#include <boost/variant/static_visitor.hpp>

namespace purplepp {
namespace detail {

/// Lambda-visitor for variant
template <typename ReturnType, typename... Lambdas>
struct lambda_visitor;

template <typename ReturnType, typename Lambda1, typename... Lambdas>
struct lambda_visitor< ReturnType, Lambda1 , Lambdas...>
		: public lambda_visitor<ReturnType, Lambdas...>, public Lambda1 {

	using Lambda1::operator();
	using lambda_visitor< ReturnType , Lambdas...>::operator();
	lambda_visitor(Lambda1 l1, Lambdas... lambdas)
			: lambda_visitor< ReturnType , Lambdas...> (lambdas...), Lambda1(l1)
	{}
};


template <typename ReturnType, typename Lambda1>
struct lambda_visitor<ReturnType, Lambda1>
		: public boost::static_visitor<ReturnType>, public Lambda1 {

	using Lambda1::operator();
	lambda_visitor(Lambda1 l1)
			: boost::static_visitor<ReturnType>(), Lambda1(l1)
	{}
};


template <typename ReturnType>
struct lambda_visitor<ReturnType>
		: public boost::static_visitor<ReturnType> {

	lambda_visitor() : boost::static_visitor<ReturnType>() {}

	//template <class T = ReturnType> std::enable_if_t<std::is_same<T, void>::value, void> operator()(boost::blank) {};
	//void operator()(boost::blank) {}
};

template <typename ReturnType = void, typename... Lambdas, class Variant>
std::enable_if_t<std::is_same<ReturnType, void>::value, ReturnType> apply_lambda_visitor(Variant&& variant, Lambdas&& ...lambdas) {
	static auto blank_visitor = [](boost::blank) {};

	auto&& visitor = lambda_visitor<ReturnType, std::decay_t<Lambdas>..., decltype(blank_visitor)>(std::forward<Lambdas>(lambdas)..., blank_visitor);
	return std::forward<Variant>(variant).apply_visitor(visitor);
};

template <typename ReturnType = void, typename... Lambdas, class Variant>
std::enable_if_t<!std::is_same<ReturnType, void>::value, ReturnType> apply_lambda_visitor(Variant&& variant, Lambdas&& ...lambdas) {
	auto&& visitor = lambda_visitor<ReturnType, std::decay_t<Lambdas>...>(std::forward<Lambdas>(lambdas)...);
	return std::forward<Variant>(variant).apply_visitor(visitor);
};

}
}