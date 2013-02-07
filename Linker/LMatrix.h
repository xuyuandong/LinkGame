// lmatrix.h
#pragma once

template < typename T, int m, int n > class Matrix
{
protected: 
	T matrix[m][n];
public:
	Matrix();
	void show_matrix(); //输出矩阵
	void set_element( int x, int y, T num ); //num为元素值
	T get_element( int x, int y );
};