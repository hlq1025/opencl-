

#include <CL/cl.h>

#include <iostream>

#include <fstream>

using namespace std;

#define MATRIX_A_ROW	8	// ����A����

#define MATRIX_A_COL	6	// ����A����

#define MATRIX_B_ROW	6	// ����B����

#define MATRIX_B_COL	8	// ����B����







// ��cl����תΪ�ַ���

cl_int ConvertToString(const char *pFileName, std::string &str);





int main()

{

	cl_int			iStatus = 0;		// ��������״̬

	cl_uint			uiNumPlatforms = 0;		// ƽ̨����

	cl_platform_id	Platform = NULL;		// ѡ���ƽ̨

	size_t			uiSize = 0;		// ƽ̨�汾�����ֽ���	

	cl_int			iErr = 0;		// ���ز���

	char			*pName = NULL;		// ƽ̨�汾��

	cl_uint			uiNumDevices = 0;		// �豸����

	cl_device_id	*pDevices = NULL;		// �豸

	cl_context		Context = NULL;		// �豸����

	cl_command_queue CommandQueue = NULL;		// �������

	const char		*pFileName = "MatrixMul.cl";	// cl�ļ���

	string			strSource = "";		// ���ڴ洢cl�ļ��еĴ���

	const char		*pSource = NULL;		// �����ַ���ָ��

	size_t			uiArrSourceSize[] = { 0 };		// �����ַ�������

	cl_program		Program = NULL;		// �������

	float			arrInMatA[MATRIX_A_ROW][MATRIX_A_COL];	// �������A

	float			arrInMatB[MATRIX_B_ROW][MATRIX_B_COL];	// �������B

	float			arrOut[MATRIX_A_ROW][MATRIX_B_COL];		// �������

	int				iMatARow = MATRIX_A_ROW;	// ����A����

	int				iMatACol = MATRIX_A_COL;	// ����A����

	int				iMatBRow = MATRIX_B_ROW;	// ����B����

	int				iMatBCol = MATRIX_B_COL;	// ����B����

	cl_mem			memInutBufferA = NULL;		// �����ڴ����A

	cl_mem			memInutBufferB = NULL;		// �����ڴ����B

	cl_mem			memOutputBuffer = NULL;		// ����ڴ����

	cl_kernel		Kernel = NULL;		// �ں˶���



	// �����趨�ں˷ֲ�����������������������	

	size_t			uiGlobal_Work_Size[2] = { MATRIX_A_ROW, MATRIX_B_COL };



	//-------------------1. ��ò�ѡ�����ƽ̨-----------------------------

	// ���ƽ̨����

	iStatus = clGetPlatformIDs(0, NULL, &uiNumPlatforms); // ��ѯ���õ�ƽ̨������������״̬

	if (CL_SUCCESS != iStatus)

	{

		cout << "Error: Getting platforms error" << endl;

		return 0;

	}





	// ���ƽ̨��ַ

	if (uiNumPlatforms > 0)  // ����п���ƽ̨

	{

		// ����ƽ̨��Ϊƽ̨�����ڴ�ռ�

		cl_platform_id *pPlatforms = (cl_platform_id *)malloc(uiNumPlatforms * sizeof(cl_platform_id));

		iStatus = clGetPlatformIDs(uiNumPlatforms, pPlatforms, NULL);		// ��ÿ��õ�ƽ̨

		Platform = pPlatforms[0];											// ��õ�һ��ƽ̨�ĵ�ַ

		free(pPlatforms);													// �ͷ�ƽ̨ռ�õ��ڴ�ռ�

	}



	// ���ƽ̨�汾��

	iErr = clGetPlatformInfo(Platform, CL_PLATFORM_VERSION, 0, NULL, &uiSize);			// ���ƽ̨�汾�����ֽ���

	pName = (char *)alloca(uiSize * sizeof(char));										// �����ֽ���Ϊƽ̨�汾�������ڴ�ռ�

	iErr = clGetPlatformInfo(Platform, CL_PLATFORM_VERSION, uiSize, pName, NULL);		// ���ƽ̨�汾����

	cout << pName << endl;





	//--------------2. ��ѯGPU�豸����ѡ������豸------------------------

	iStatus = clGetDeviceIDs(Platform, CL_DEVICE_TYPE_GPU, 0, NULL, &uiNumDevices);		// ���GPU�豸����

	if (0 == uiNumDevices)			// ���û��GPU�豸

	{

		cout << "No GPU device available." << endl;

		cout << "Choose CPU as default device." << endl;

		iStatus = clGetDeviceIDs(Platform, CL_DEVICE_TYPE_CPU, 0, NULL, &uiNumDevices);  // ѡ��CPU��Ϊ�豸������豸��

		pDevices = (cl_device_id *)malloc(uiNumDevices * sizeof(cl_device_id));			 // Ϊ�豸����ռ�



		iStatus = clGetDeviceIDs(Platform, CL_DEVICE_TYPE_CPU, uiNumDevices, pDevices, NULL);  // ���ƽ̨

	}

	else

	{

		pDevices = (cl_device_id *)malloc(uiNumDevices * sizeof(cl_device_id));



		iStatus = clGetDeviceIDs(Platform, CL_DEVICE_TYPE_GPU, uiNumDevices, pDevices, NULL);

	}





	// -------------------3.�����豸����---------------------------------

	Context = clCreateContext(NULL, 1, pDevices, NULL, NULL, NULL);	// �����豸����

	if (NULL == Context)

	{

		cout << "Error: Can not create context" << endl;

		return 0;

	}



	// -------------------4.�����������--------------------------------------

	CommandQueue = clCreateCommandQueue(Context, pDevices[0], 0, NULL);	// ������1���豸���������

	if (NULL == CommandQueue)

	{

		cout << "Error: Can not create CommandQueue" << endl;

		return 0;

	}



	// ----------------------5. �����������------------------------------

	iStatus = ConvertToString(pFileName, strSource);// ��cl�ļ��еĴ���תΪ�ַ���

	pSource = strSource.c_str();		// ���strSourceָ��

	uiArrSourceSize[0] = strlen(pSource);	// �ַ�����С

	Program = clCreateProgramWithSource(Context, 1, &pSource, uiArrSourceSize, NULL);	// �����������

	if (NULL == Program)

	{

		cout << "Error: Can not create program" << endl;

		return 0;

	}



	// -----------------------------6. �������--------------------------------

	iStatus = clBuildProgram(Program, 1, pDevices, NULL, NULL, NULL);	// �������

	if (CL_SUCCESS != iStatus)	 // �������

	{

		cout << "Error: Can not build program" << endl;

		char szBuildLog[16384];

		clGetProgramBuildInfo(Program, *pDevices, CL_PROGRAM_BUILD_LOG, sizeof(szBuildLog), szBuildLog, NULL);



		cout << "Error in Kernel: " << endl << szBuildLog;

		clReleaseProgram(Program);



		return 0;

	}



	//-------------------------7. ��������������ں��ڴ����--------------------------------

	// ���þ���A����

	for (int i = 0; i<MATRIX_A_ROW; i++)

	{

		for (int j = 0; j<MATRIX_A_COL; j++)

		{

			arrInMatA[i][j] = i + j;

		}

	}



	// ���þ���B����

	for (int i = 0; i<MATRIX_B_ROW; i++)

	{

		for (int j = 0; j<MATRIX_B_COL; j++)

		{

			arrInMatB[i][j] = i - j;

		}

	}



	// ���������ڴ����A

	memInutBufferA = clCreateBuffer(

		Context,

		CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,  // �����ڴ�Ϊֻ���������Դ��������ڴ渴�Ƶ��豸�ڴ�

		MATRIX_A_ROW * MATRIX_A_COL * sizeof(float),	 // �����ڴ�ռ��С

		(void *)arrInMatA,

		NULL);



	// ���������ڴ����B

	memInutBufferB = clCreateBuffer(

		Context,

		CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,  // �����ڴ�Ϊֻ���������Դ��������ڴ渴�Ƶ��豸�ڴ�

		MATRIX_B_ROW * MATRIX_B_COL * sizeof(float), // �����ڴ�ռ��С

		(void *)arrInMatB,

		NULL);



	// ��������ڴ����

	memOutputBuffer = clCreateBuffer(

		Context,

		CL_MEM_WRITE_ONLY,							// ����ڴ�ֻ��д

		MATRIX_A_ROW * MATRIX_B_COL * sizeof(int),	// ����ڴ�ռ��С

		NULL,

		NULL);



	if ((NULL == memInutBufferA) || (NULL == memInutBufferB) || (NULL == memOutputBuffer))

	{

		cout << "Error creating memory objects" << endl;

		return 0;

	}



	//--------------------------8. �����ں˶���-------------------------------------

	Kernel = clCreateKernel(Program,

		"MatrixMul",  // cl�ļ��е���ں���

		NULL);

	if (NULL == Kernel)

	{

		cout << "Error: Can not create kernel" << endl;

		return 0;

	}



	//----------------------------9. �����ں˲���----------------------------------

	iStatus |= clSetKernelArg(Kernel, 0, sizeof(cl_int), (void *)&(iMatARow));	// ����A����

	iStatus |= clSetKernelArg(Kernel, 1, sizeof(cl_int), (void *)&(iMatACol));	// ����A����

	iStatus |= clSetKernelArg(Kernel, 2, sizeof(cl_mem), (void *)&memInutBufferA); // ����A



	iStatus |= clSetKernelArg(Kernel, 3, sizeof(cl_int), (void *)&(iMatBRow));	// ����B����

	iStatus |= clSetKernelArg(Kernel, 4, sizeof(cl_int), (void *)&(iMatBCol));	// ����B����	

	iStatus |= clSetKernelArg(Kernel, 5, sizeof(cl_mem), (void *)&memInutBufferB); // ����B	



	iStatus |= clSetKernelArg(Kernel, 6, sizeof(cl_mem), (void *)&memOutputBuffer); // �������



	if (CL_SUCCESS != iStatus)

	{

		cout << "Error setting kernel arguments" << endl;

	}



	// --------------------------10.�����ں�---------------------------------

	// �����������ʹ�����豸��ִ�е��ں��Ŷ�

	iStatus = clEnqueueNDRangeKernel(

		CommandQueue,

		Kernel,

		2,

		NULL,

		uiGlobal_Work_Size,  // �ں˷ֲ�(����������)

		NULL,

		0,

		NULL,

		NULL);



	if (CL_SUCCESS != iStatus)

	{

		cout << "Error: Can not run kernel" << endl;

		return 0;

	}



	// ----------------------------11. �������ȡ�������ڴ�

	iStatus = clEnqueueReadBuffer(

		CommandQueue,		// �������

		memOutputBuffer,	// ����ڴ����

		CL_TRUE,			// �ں˶�ȡ����֮ǰ�ú������᷵��

		0,

		MATRIX_A_ROW * MATRIX_B_COL * sizeof(float),

		arrOut,

		0,

		NULL,

		NULL);



	if (CL_SUCCESS != iStatus)

	{

		cout << "Error: Can not reading result buffer" << endl;

		return 0;

	}



	// ---------------------12--���������---------------

	cout << "Matrix A(" << MATRIX_A_ROW << "x" << MATRIX_A_COL << "): is" << endl;

	for (int i = 0; i<MATRIX_A_ROW; i++)

	{

		for (int j = 0; j<MATRIX_A_COL; j++)

		{

			printf("%4.0f  ", arrInMatA[i][j]);

		}

		cout << endl;

	}

	cout << endl;



	cout << "Matrix B(" << MATRIX_B_ROW << "x" << MATRIX_B_COL << "): is" << endl;

	for (int i = 0; i<MATRIX_B_ROW; i++)

	{

		for (int j = 0; j<MATRIX_B_COL; j++)

		{

			printf("%4.0f  ", arrInMatB[i][j]);

		}

		cout << endl;

	}

	cout << endl;



	cout << "Output Matrix C(" << MATRIX_A_ROW << "x" << MATRIX_B_COL << "): is" << endl;

	for (int i = 0; i<MATRIX_A_ROW; i++)

	{

		for (int j = 0; j<MATRIX_B_COL; j++)

		{

			printf("%4.0f  ", arrOut[i][j]);

		}

		cout << endl;

	}



	// -------------------------------13. �ͷ���Դ--------------------------------

	iStatus = clReleaseKernel(Kernel);

	iStatus = clReleaseProgram(Program);

	iStatus = clReleaseMemObject(memInutBufferA);

	iStatus = clReleaseMemObject(memInutBufferB);

	iStatus = clReleaseMemObject(memOutputBuffer);

	iStatus = clReleaseCommandQueue(CommandQueue);

	iStatus = clReleaseContext(Context);



	if (NULL != pDevices)

	{

		free(pDevices);

		pDevices = NULL;

	}



	return 0;

}





/************************************************************************/

/*

���ܣ�  ��cl����תΪ�ַ���

���룺 �ļ����� �ַ�����ַ

����� �ַ���

���أ� �ɹ�����0�� ʧ�ܷ���-1

*/

/************************************************************************/

cl_int ConvertToString(const char *pFileName, std::string &Str)

{

	size_t		uiSize = 0;

	size_t		uiFileSize = 0;

	char		*pStr = NULL;

	std::fstream fFile(pFileName, (std::fstream::in | std::fstream::binary));





	if (fFile.is_open())

	{

		fFile.seekg(0, std::fstream::end);

		uiSize = uiFileSize = (size_t)fFile.tellg();  // ����ļ���С

		fFile.seekg(0, std::fstream::beg);

		pStr = new char[uiSize + 1];



		if (NULL == pStr)

		{

			fFile.close();

			return 0;

		}



		fFile.read(pStr, uiFileSize);				// ��ȡuiFileSize�ֽ�

		fFile.close();

		pStr[uiSize] = '\0';

		Str = pStr;



		delete[] pStr;



		return 0;

	}



	cout << "Error: Failed to open cl file\n:" << pFileName << endl;

	return -1;

}
