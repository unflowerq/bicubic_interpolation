#include <iostream>
#include <fstream>

using namespace std;

void MemFree2D(unsigned char **Mem, int nHeight)
{
	for (int n = 0; n < nHeight; n++)
	{
		delete[] Mem[n];
	}
	delete[] Mem;
}

unsigned char** MemAlloc2D(int nHeight, int nWidth, unsigned char nInitVal)
{
	unsigned char** rtn = new unsigned char*[nHeight];
	for (int n = 0; n < nHeight; n++)
	{
		rtn[n] = new unsigned char[nWidth];
		memset(rtn[n], nInitVal, sizeof(unsigned char) * nWidth);
	}
	return rtn;
}

bool isInsideBoundary(int nHeight, int nWidth, double h, double w)
{
	if (h >= 0 && w >= 0 && h < nHeight && w < nWidth)
	{
		return true;
	}
	else return false;
}

unsigned char NoInterpolation(unsigned char** In, int nHeight_Ori, int nWidth_Ori, double h_Cvt, double w_Cvt)
{
	unsigned char r1, r2, r3, r4;
	unsigned char return_value;
	int h1, w1, h2, w2, h3, w3, h4, w4;

	h1 = floor(h_Cvt); w1 = floor(w_Cvt);
	h2 = h1; w2 = w1 + 1;
	h3 = h1 + 1; w3 = w1;
	h4 = h1 + 1; w4 = w1 + 1;

	if (h1 == nHeight_Ori - 1 && w1 == nWidth_Ori - 1)
		return 0;
	else if (h1 == nHeight_Ori - 1 && w1 != nWidth_Ori - 1)
		return 0;
	else if (h1 != nHeight_Ori - 1 && w1 == nWidth_Ori - 1)
		return 0;
	else
	{
		return_value = In[h1][w1];

		return return_value;
	}
}

unsigned char BicubicInterpolation(unsigned char** In, int nHeight_Ori, int nWidth_Ori, double h_Cvt, double w_Cvt)
{
	unsigned char return_value;
	int h1, w1;

	h1 = floor(h_Cvt);
	w1 = floor(w_Cvt);

	double real_w = w_Cvt - w1;
	double real_h = h_Cvt - h1;

	if (h1 == 0)
		return 0;
	else if (w1 == 0)
		return 0;
	else if (w1 > nWidth_Ori - 3)
		return 0;
	else if (h1 > nHeight_Ori - 3)
		return 0;
	else
	{
		double reverse_matrix[4][4] = { { -0.166667, 0.5, -0.5, 0.166667 },{ 0.5, -1, 0.5, 0 },{ -0.333333, -0.5, 1, -0.166667 },{ 0, 1, 0, 0 } };

		double onefourfourfour[4];
		double findvalue[4];
		double onefourfourone[4];

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				onefourfourfour[j] = reverse_matrix[j][0] * In[(h1 - 1) + i][w1 - 1] + reverse_matrix[j][1] * In[(h1 - 1) + i][w1] + reverse_matrix[j][2] * In[(h1 - 1) + i][w1 + 1] + reverse_matrix[j][3] * In[(h1 - 1) + i][w1 + 2];
			}
			findvalue[i] = real_w*real_w*real_w*onefourfourfour[0] + real_w*real_w*onefourfourfour[1] + real_w*onefourfourfour[2] + onefourfourfour[3];
		}

		for (int i = 0; i < 4; i++)
		{
			onefourfourone[i] = reverse_matrix[i][0] * findvalue[0] + reverse_matrix[i][1] * findvalue[1] + reverse_matrix[i][2] * findvalue[2] + reverse_matrix[i][3] * findvalue[3];
		}

		return_value = real_h*real_h*real_h*onefourfourone[0] + real_h*real_h*onefourfourone[1] + real_h*onefourfourone[2] + onefourfourone[3];

		return return_value;
	}
}

int main()
{
	int width = 256;
	int height = 256;
	int input_angle;

	cout << "interpolation + Rotation" << endl << "Enter the angle : ";
	cin >> input_angle;

	unsigned char **output_img_rot;
	unsigned char **output_img_rotation_bicubic;
	output_img_rot = MemAlloc2D(256, 256, 0);
	output_img_rotation_bicubic = MemAlloc2D(256, 256, 0);

	FILE *inFile;
	fopen_s(&inFile, "lena256.raw", "rb");

	if (inFile == NULL)
	{
		cout << "input image error";
		return 0;
	}

	unsigned char **input_img;

	input_img = MemAlloc2D(width, height, 0);

	for (int h = 0; h < height; h++)
	{
		fread(input_img[h], sizeof(unsigned char), width, inFile);
	}

	FILE *outFileRot;
	fopen_s(&outFileRot, "lena_cvt_rot.raw", "wb");

	FILE *outFileRotBicub;
	fopen_s(&outFileRotBicub, "lena_ctr_rot_bicub.raw", "wb");

	///////////////rotation, rotation + bicubic
	if (input_angle < 0 || input_angle > 360)
	{
		cout << "angle error";
	}

	int nHeight_2 = 256 / 2;
	int nWidth_2 = 256 / 2;

	double h_Rotate = 0;
	double w_Rotate = 0;

	double sin_value = (double)sin(input_angle * (3.14 / 180));
	double cos_value = (double)cos(input_angle * (3.14 / 180));

	for (int h = 0; h < 256; h++)
	{
		for (int w = 0; w < 256; w++)
		{
			int rot_h = h - nHeight_2;
			int rot_w = w - nWidth_2;

			h_Rotate = sin_value*rot_w + cos_value*rot_h + nHeight_2;
			w_Rotate = cos_value*rot_w - sin_value*rot_h + nWidth_2;

			if (isInsideBoundary(256, 256, h_Rotate, w_Rotate))
				output_img_rot[h][w] = NoInterpolation(input_img, 256, 256, h_Rotate, w_Rotate);
		}
	}

	for (int h = 0; h < 256; h++)
	{
		for (int w = 0; w < 256; w++)
		{
			int rot_h = h - nHeight_2;
			int rot_w = w - nWidth_2;

			h_Rotate = sin_value*rot_w + cos_value*rot_h + nHeight_2;
			w_Rotate = cos_value*rot_w - sin_value*rot_h + nWidth_2;

			if (isInsideBoundary(256, 256, h_Rotate, w_Rotate))
				output_img_rotation_bicubic[h][w] = BicubicInterpolation(input_img, 256, 256, h_Rotate, w_Rotate);
		}
	}

	//////////////////파일에 rotation 쓰기
	for (int h = 0; h < 256; h++)
	{
		fwrite(output_img_rot[h], sizeof(unsigned char), 256, outFileRot);
	}

	/////////////////파일에 rotation + bicubic 쓰기
	for (int h = 0; h < 256; h++)
	{
		fwrite(output_img_rotation_bicubic[h], sizeof(unsigned char), 256, outFileRotBicub);
	}

	MemFree2D(input_img, 256);
	MemFree2D(output_img_rot, 256);
	MemFree2D(output_img_rotation_bicubic, 256);
	fclose(inFile);
	fclose(outFileRot);
	fclose(outFileRotBicub);

	return 0;
}