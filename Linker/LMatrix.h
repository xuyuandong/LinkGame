// lmatrix.h
#pragma once

template < typename T, int m, int n > class Matrix
{
protected: 
	T matrix[m][n];
public:
	Matrix();
	void show_matrix(); //�������
	void set_element( int x, int y, T num ); //numΪԪ��ֵ
	T get_element( int x, int y );
};