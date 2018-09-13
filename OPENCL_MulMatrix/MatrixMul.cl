
__kernel void MatrixMul(

	int iHeightA,					// 矩阵A行数

	int iWidthA,					// 矩阵A列数

	__global float *pInMatA,		// 矩阵A

	int iHeightB,					// 矩阵B行数

	int iWidthB,					// 矩阵B列数

	__global float *pInMatB,		// 矩阵B

	__global float *pOutMat)		// 输出矩阵

{

	int iRow = get_global_id(0);		// 当前行索引

	int iCol = get_global_id(1);		// 当前列索引



	float fSum = 0.0f;



	for (int i = 0; i< iWidthA; i++)

	{

		fSum += pInMatA[iRow * iWidthA + i] * pInMatB[iWidthB * i + iCol];

	}



	pOutMat[iRow * iWidthB + iCol] = fSum;

}
