
__kernel void MatrixMul(

	int iHeightA,					// ����A����

	int iWidthA,					// ����A����

	__global float *pInMatA,		// ����A

	int iHeightB,					// ����B����

	int iWidthB,					// ����B����

	__global float *pInMatB,		// ����B

	__global float *pOutMat)		// �������

{

	int iRow = get_global_id(0);		// ��ǰ������

	int iCol = get_global_id(1);		// ��ǰ������



	float fSum = 0.0f;



	for (int i = 0; i< iWidthA; i++)

	{

		fSum += pInMatA[iRow * iWidthA + i] * pInMatB[iWidthB * i + iCol];

	}



	pOutMat[iRow * iWidthB + iCol] = fSum;

}
