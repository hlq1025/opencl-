

#include <CL/cl.h>

#include <iostream>

#include <fstream>

using namespace std;

#define MATRIX_A_ROW	8	// 矩阵A行数

#define MATRIX_A_COL	6	// 矩阵A列数

#define MATRIX_B_ROW	6	// 矩阵B行数

#define MATRIX_B_COL	8	// 矩阵B列数







// 将cl代码转为字符串

cl_int ConvertToString(const char *pFileName, std::string &str);





int main()

{

	cl_int			iStatus = 0;		// 函数返回状态

	cl_uint			uiNumPlatforms = 0;		// 平台个数

	cl_platform_id	Platform = NULL;		// 选择的平台

	size_t			uiSize = 0;		// 平台版本名字字节数	

	cl_int			iErr = 0;		// 返回参数

	char			*pName = NULL;		// 平台版本名

	cl_uint			uiNumDevices = 0;		// 设备数量

	cl_device_id	*pDevices = NULL;		// 设备

	cl_context		Context = NULL;		// 设备环境

	cl_command_queue CommandQueue = NULL;		// 命令队列

	const char		*pFileName = "MatrixMul.cl";	// cl文件名

	string			strSource = "";		// 用于存储cl文件中的代码

	const char		*pSource = NULL;		// 代码字符串指针

	size_t			uiArrSourceSize[] = { 0 };		// 代码字符串长度

	cl_program		Program = NULL;		// 程序对象

	float			arrInMatA[MATRIX_A_ROW][MATRIX_A_COL];	// 输入矩阵A

	float			arrInMatB[MATRIX_B_ROW][MATRIX_B_COL];	// 输入矩阵B

	float			arrOut[MATRIX_A_ROW][MATRIX_B_COL];		// 输出矩阵

	int				iMatARow = MATRIX_A_ROW;	// 矩阵A行数

	int				iMatACol = MATRIX_A_COL;	// 矩阵A列数

	int				iMatBRow = MATRIX_B_ROW;	// 矩阵B行数

	int				iMatBCol = MATRIX_B_COL;	// 矩阵B列数

	cl_mem			memInutBufferA = NULL;		// 输入内存对象A

	cl_mem			memInutBufferB = NULL;		// 输入内存对象B

	cl_mem			memOutputBuffer = NULL;		// 输出内存对象

	cl_kernel		Kernel = NULL;		// 内核对象



	// 用于设定内核分布，即输出据真的行数，列数	

	size_t			uiGlobal_Work_Size[2] = { MATRIX_A_ROW, MATRIX_B_COL };



	//-------------------1. 获得并选择可用平台-----------------------------

	// 获得平台数量

	iStatus = clGetPlatformIDs(0, NULL, &uiNumPlatforms); // 查询可用的平台个数，并返回状态

	if (CL_SUCCESS != iStatus)

	{

		cout << "Error: Getting platforms error" << endl;

		return 0;

	}





	// 获得平台地址

	if (uiNumPlatforms > 0)  // 如果有可用平台

	{

		// 根据平台数为平台分配内存空间

		cl_platform_id *pPlatforms = (cl_platform_id *)malloc(uiNumPlatforms * sizeof(cl_platform_id));

		iStatus = clGetPlatformIDs(uiNumPlatforms, pPlatforms, NULL);		// 获得可用的平台

		Platform = pPlatforms[0];											// 获得第一个平台的地址

		free(pPlatforms);													// 释放平台占用的内存空间

	}



	// 获得平台版本名

	iErr = clGetPlatformInfo(Platform, CL_PLATFORM_VERSION, 0, NULL, &uiSize);			// 获得平台版本名的字节数

	pName = (char *)alloca(uiSize * sizeof(char));										// 根据字节数为平台版本名分配内存空间

	iErr = clGetPlatformInfo(Platform, CL_PLATFORM_VERSION, uiSize, pName, NULL);		// 获得平台版本名字

	cout << pName << endl;





	//--------------2. 查询GPU设备，并选择可用设备------------------------

	iStatus = clGetDeviceIDs(Platform, CL_DEVICE_TYPE_GPU, 0, NULL, &uiNumDevices);		// 获得GPU设备数量

	if (0 == uiNumDevices)			// 如果没有GPU设备

	{

		cout << "No GPU device available." << endl;

		cout << "Choose CPU as default device." << endl;

		iStatus = clGetDeviceIDs(Platform, CL_DEVICE_TYPE_CPU, 0, NULL, &uiNumDevices);  // 选择CPU作为设备，获得设备数

		pDevices = (cl_device_id *)malloc(uiNumDevices * sizeof(cl_device_id));			 // 为设备分配空间



		iStatus = clGetDeviceIDs(Platform, CL_DEVICE_TYPE_CPU, uiNumDevices, pDevices, NULL);  // 获得平台

	}

	else

	{

		pDevices = (cl_device_id *)malloc(uiNumDevices * sizeof(cl_device_id));



		iStatus = clGetDeviceIDs(Platform, CL_DEVICE_TYPE_GPU, uiNumDevices, pDevices, NULL);

	}





	// -------------------3.创建设备环境---------------------------------

	Context = clCreateContext(NULL, 1, pDevices, NULL, NULL, NULL);	// 创建设备环境

	if (NULL == Context)

	{

		cout << "Error: Can not create context" << endl;

		return 0;

	}



	// -------------------4.创建命令队列--------------------------------------

	CommandQueue = clCreateCommandQueue(Context, pDevices[0], 0, NULL);	// 创建第1个设备的命令队列

	if (NULL == CommandQueue)

	{

		cout << "Error: Can not create CommandQueue" << endl;

		return 0;

	}



	// ----------------------5. 创建程序对象------------------------------

	iStatus = ConvertToString(pFileName, strSource);// 将cl文件中的代码转为字符串

	pSource = strSource.c_str();		// 获得strSource指针

	uiArrSourceSize[0] = strlen(pSource);	// 字符串大小

	Program = clCreateProgramWithSource(Context, 1, &pSource, uiArrSourceSize, NULL);	// 创建程序对象

	if (NULL == Program)

	{

		cout << "Error: Can not create program" << endl;

		return 0;

	}



	// -----------------------------6. 编译程序--------------------------------

	iStatus = clBuildProgram(Program, 1, pDevices, NULL, NULL, NULL);	// 编译程序

	if (CL_SUCCESS != iStatus)	 // 编译错误

	{

		cout << "Error: Can not build program" << endl;

		char szBuildLog[16384];

		clGetProgramBuildInfo(Program, *pDevices, CL_PROGRAM_BUILD_LOG, sizeof(szBuildLog), szBuildLog, NULL);



		cout << "Error in Kernel: " << endl << szBuildLog;

		clReleaseProgram(Program);



		return 0;

	}



	//-------------------------7. 并创建输入输出内核内存对象--------------------------------

	// 设置矩阵A数据

	for (int i = 0; i<MATRIX_A_ROW; i++)

	{

		for (int j = 0; j<MATRIX_A_COL; j++)

		{

			arrInMatA[i][j] = i + j;

		}

	}



	// 设置矩阵B数据

	for (int i = 0; i<MATRIX_B_ROW; i++)

	{

		for (int j = 0; j<MATRIX_B_COL; j++)

		{

			arrInMatB[i][j] = i - j;

		}

	}



	// 创建输入内存对象A

	memInutBufferA = clCreateBuffer(

		Context,

		CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,  // 输入内存为只读，并可以从宿主机内存复制到设备内存

		MATRIX_A_ROW * MATRIX_A_COL * sizeof(float),	 // 输入内存空间大小

		(void *)arrInMatA,

		NULL);



	// 创建输入内存对象B

	memInutBufferB = clCreateBuffer(

		Context,

		CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,  // 输入内存为只读，并可以从宿主机内存复制到设备内存

		MATRIX_B_ROW * MATRIX_B_COL * sizeof(float), // 输入内存空间大小

		(void *)arrInMatB,

		NULL);



	// 创建输出内存对象

	memOutputBuffer = clCreateBuffer(

		Context,

		CL_MEM_WRITE_ONLY,							// 输出内存只能写

		MATRIX_A_ROW * MATRIX_B_COL * sizeof(int),	// 输出内存空间大小

		NULL,

		NULL);



	if ((NULL == memInutBufferA) || (NULL == memInutBufferB) || (NULL == memOutputBuffer))

	{

		cout << "Error creating memory objects" << endl;

		return 0;

	}



	//--------------------------8. 创建内核对象-------------------------------------

	Kernel = clCreateKernel(Program,

		"MatrixMul",  // cl文件中的入口函数

		NULL);

	if (NULL == Kernel)

	{

		cout << "Error: Can not create kernel" << endl;

		return 0;

	}



	//----------------------------9. 设置内核参数----------------------------------

	iStatus |= clSetKernelArg(Kernel, 0, sizeof(cl_int), (void *)&(iMatARow));	// 矩阵A行数

	iStatus |= clSetKernelArg(Kernel, 1, sizeof(cl_int), (void *)&(iMatACol));	// 矩阵A列数

	iStatus |= clSetKernelArg(Kernel, 2, sizeof(cl_mem), (void *)&memInutBufferA); // 矩阵A



	iStatus |= clSetKernelArg(Kernel, 3, sizeof(cl_int), (void *)&(iMatBRow));	// 矩阵B行数

	iStatus |= clSetKernelArg(Kernel, 4, sizeof(cl_int), (void *)&(iMatBCol));	// 矩阵B列数	

	iStatus |= clSetKernelArg(Kernel, 5, sizeof(cl_mem), (void *)&memInutBufferB); // 矩阵B	



	iStatus |= clSetKernelArg(Kernel, 6, sizeof(cl_mem), (void *)&memOutputBuffer); // 输出矩阵



	if (CL_SUCCESS != iStatus)

	{

		cout << "Error setting kernel arguments" << endl;

	}



	// --------------------------10.运行内核---------------------------------

	// 利用命令队列使将再设备上执行的内核排队

	iStatus = clEnqueueNDRangeKernel(

		CommandQueue,

		Kernel,

		2,

		NULL,

		uiGlobal_Work_Size,  // 内核分布(行数，列数)

		NULL,

		0,

		NULL,

		NULL);



	if (CL_SUCCESS != iStatus)

	{

		cout << "Error: Can not run kernel" << endl;

		return 0;

	}



	// ----------------------------11. 将输出读取到主机内存

	iStatus = clEnqueueReadBuffer(

		CommandQueue,		// 命令队列

		memOutputBuffer,	// 输出内存对象

		CL_TRUE,			// 内核读取结束之前该函数不会返回

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



	// ---------------------12--输出计算结果---------------

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



	// -------------------------------13. 释放资源--------------------------------

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

功能：  将cl代码转为字符串

输入： 文件名， 字符串地址

输出： 字符串

返回： 成功返回0， 失败返回-1

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

		uiSize = uiFileSize = (size_t)fFile.tellg();  // 获得文件大小

		fFile.seekg(0, std::fstream::beg);

		pStr = new char[uiSize + 1];



		if (NULL == pStr)

		{

			fFile.close();

			return 0;

		}



		fFile.read(pStr, uiFileSize);				// 读取uiFileSize字节

		fFile.close();

		pStr[uiSize] = '\0';

		Str = pStr;



		delete[] pStr;



		return 0;

	}



	cout << "Error: Failed to open cl file\n:" << pFileName << endl;

	return -1;

}
