#include "stdafx.h"
#include "LMatrix.h"

template < typename T, int m, int n > 
Matrix < T, m, n >::Matrix()
{
 for( int i=0; i<m; i++ )
  for( int j=0; j<n; j++ )
   matrix[i][j] = -1;
}

template < typename T, int m, int n > 
void Matrix< T, m, n >::show_matrix()
{ 
	for( int i=0; i<m; i++ )
	{
		for( int j=0; j<n; j++ )
			printf( "%s\t", matrix[i][j] );
		printf("\n");
	}
}

template < typename T, int m, int n > 
inline void Matrix< T, m, n >::set_element( int x, int y, T element )
{
	matrix[x][y] = element;
}

template < typename T, int m, int n > 
T Matrix< T, m, n >::get_element( int x, int y )
{
	return matrix[x][y];
}