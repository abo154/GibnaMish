#pragma once
#ifndef _TABLE_HPP_
#define _TABLE_HPP_

#include <cstdint>
#include <array>

template <typename T, size_t N, size_t... Dims>
struct Table
{
	Table();
	Table<T, Dims...>& operator[](size_t);
	const Table<T, Dims...>& operator[](size_t) const;
	void reset();
private:
	std::array<Table<T, Dims...>, N> data;
};

template <typename T, size_t N>
struct Table<T, N>
{
	Table();
	T& operator[](size_t index);
	const T& operator[](size_t index) const;
	void reset();
private:
	std::array<T, N> data;
};

template <typename T, size_t N>
Table<T, N>::Table()
{
	data.fill({});
}

template <typename T, size_t N>
T& Table<T, N>::operator[](size_t index)
{
	return data[index];
}

template <typename T, size_t N>
const T& Table<T, N>::operator[](size_t index) const
{
	return data[index];
}

template <typename T, size_t N>
void Table<T, N>::reset()
{
	data.fill({});
}


template <typename T, size_t N, size_t... Dims>
Table<T, N, Dims...>::Table()
{
	data.fill({});
}

template <typename T, size_t N, size_t... Dims>
Table<T, Dims...>& Table<T, N, Dims...>::operator[](size_t index)
{
	return data[index];
}

template <typename T, size_t N, size_t... Dims>
const Table<T, Dims...>& Table<T, N, Dims...>::operator[](size_t index) const
{
	return data[index];
}

template <typename T, size_t N, size_t... Dims>
void Table<T, N, Dims...>::reset()
{
	data.fill({});
}

#endif // !_TABLE_HPP_