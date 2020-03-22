#ifndef _ZQ_HEAD_POSE_ESTIMATION_H_
#define _ZQ_HEAD_POSE_ESTIMATION_H_
#pragma once

#include "ZQlib/ZQ_DoubleImage.h"
#include "ZQlib/ZQ_MathBase.h"
#include "ZQlib/ZQ_Rodrigues.h"
#include "ZQlib/ZQ_LevMar.h"
#include <string.h>

namespace ZQ
{
	class ZQ_HeadPoseEstimation
	{
	public:
		static void ComputeYawPicthRoll(int width, int height, float focal, const float* landmarks, float& yaw, float& pitch, float &roll,
			int max_iter = 3, bool use_levmar = false)
		{
			float X3[14 * 3] =
			{
				-6.825897, -6.760612, -4.402142,
				-1.330353, -7.122144, -6.903745,
				1.330353, -7.122144, -6.903745,
				6.825897, -6.760612, -4.402142,
				-5.311432, -5.485328, -3.987654,
				-1.789930, -5.393625, -4.413414,
				1.789930, -5.393625, -4.413414,
				5.311432, -5.485328, -3.987654,
				-2.005628, -1.409845, -6.165652,
				2.005628, -1.409845, -6.165652,
				-2.774015, 2.080775, -5.048531,
				2.774015, 2.080775, -5.048531,
				0.000000, 3.116408, -6.097667,
				0.000000, 7.415691, -4.070434
			};

			float X2[14 * 2] =
			{
				landmarks[33 * 2 + 0],landmarks[33 * 2 + 1],
				landmarks[37 * 2 + 0],landmarks[37 * 2 + 1],
				landmarks[38 * 2 + 0],landmarks[38 * 2 + 1],
				landmarks[42 * 2 + 0],landmarks[42 * 2 + 1],
				landmarks[52 * 2 + 0],landmarks[52 * 2 + 1],
				landmarks[55 * 2 + 0],landmarks[55 * 2 + 1],
				landmarks[58 * 2 + 0],landmarks[58 * 2 + 1],
				landmarks[61 * 2 + 0],landmarks[61 * 2 + 1],
				landmarks[47 * 2 + 0],landmarks[47 * 2 + 1],
				landmarks[51 * 2 + 0],landmarks[51 * 2 + 1],
				landmarks[84 * 2 + 0],landmarks[84 * 2 + 1],
				landmarks[90 * 2 + 0],landmarks[90 * 2 + 1],
				landmarks[93 * 2 + 0],landmarks[93 * 2 + 1],
				landmarks[16 * 2 + 0],landmarks[16 * 2 + 1]
			};

			float fc[2] = { focal, focal };
			float cc[2] = { 0.5*width, 0.5*height };
			float rT[6];
			HeadPoseEstimation(14, X2, X3, fc, cc, rT, true, max_iter, use_levmar);
			float R[9];
			ZQ_Rodrigues::ZQ_Rodrigues_r2R(rT, R);
			EulerAngleFromRotationMatrix(R, yaw, pitch, roll);
		}

		static bool HeadPoseEstimation(int nPts, const float* X2, const float* X3, const float fc[2], const float cc[2],
			float rT[6], bool zAxis_in, int max_iter /* = 20*/, bool use_levmar = false)
		{
			const double eps = 1e-16;
			// Computes an initial guess for extrinsic parameters(works for general 3d structure, not planar!!!) :
			// The DLT method is applied here!!
			ZQ_DImage<float> Xn_mat(nPts * 2, 1);
			ZQ_DImage<float> Y_mat(nPts * 3, 1);
			float*& Xn = Xn_mat.data();
			float*& Y = Y_mat.data();

			for (int i = 0; i < nPts; i++)
			{
				// First: Subtract principal point, and divide by the focal length :
				Xn[i * 2 + 0] = (X2[i * 2 + 0] - cc[0]) / fc[0];
				Xn[i * 2 + 1] = (X2[i * 2 + 1] - cc[1]) / fc[1];
			}

			ZQ_DImage<double> J_mat(2 * nPts, 12);
			double*& J = J_mat.data();
			double axis_flag = zAxis_in ? -1 : 1;
			for (int i = 0; i < nPts; i++)
			{
				double xX[4] = { Xn[i * 2 + 0] * X3[i * 3 + 0], Xn[i * 2 + 0] * X3[i * 3 + 1], Xn[i * 2 + 0] * X3[i * 3 + 2], Xn[i * 2 + 0] };
				double yX[4] = { Xn[i * 2 + 1] * X3[i * 3 + 0], Xn[i * 2 + 1] * X3[i * 3 + 1], Xn[i * 2 + 1] * X3[i * 3 + 2],Xn[i * 2 + 1] };
				J[(i * 2 + 0) * 12 + 0] = X3[i * 3 + 0];
				J[(i * 2 + 0) * 12 + 1] = X3[i * 3 + 1];
				J[(i * 2 + 0) * 12 + 2] = X3[i * 3 + 2];
				J[(i * 2 + 0) * 12 + 3] = 1;
				J[(i * 2 + 0) * 12 + 4] = 0;
				J[(i * 2 + 0) * 12 + 5] = 0;
				J[(i * 2 + 0) * 12 + 6] = 0;
				J[(i * 2 + 0) * 12 + 7] = 0;
				J[(i * 2 + 0) * 12 + 8] = xX[0] * axis_flag;
				J[(i * 2 + 0) * 12 + 9] = xX[1] * axis_flag;
				J[(i * 2 + 0) * 12 + 10] = xX[2] * axis_flag;
				J[(i * 2 + 0) * 12 + 11] = xX[3] * axis_flag;
				J[(i * 2 + 1) * 12 + 0] = 0;
				J[(i * 2 + 1) * 12 + 1] = 0;
				J[(i * 2 + 1) * 12 + 2] = 0;
				J[(i * 2 + 1) * 12 + 3] = 0;
				J[(i * 2 + 1) * 12 + 4] = X3[i * 3 + 0];
				J[(i * 2 + 1) * 12 + 5] = X3[i * 3 + 1];
				J[(i * 2 + 1) * 12 + 6] = X3[i * 3 + 2];
				J[(i * 2 + 1) * 12 + 7] = 1;
				J[(i * 2 + 1) * 12 + 8] = yX[0] * axis_flag;
				J[(i * 2 + 1) * 12 + 9] = yX[1] * axis_flag;
				J[(i * 2 + 1) * 12 + 10] = yX[2] * axis_flag;
				J[(i * 2 + 1) * 12 + 11] = yX[3] * axis_flag;
			}

			ZQ_DImage<double> JJ_mat(12, 12);
			double*& JJ = JJ_mat.data();
			for (int i = 0; i < 12; i++)
			{
				for (int j = 0; j < 12; j++)
				{
					JJ[i * 12 + j] = 0;
					for (int k = 0; k < 2 * nPts; k++)
						JJ[i * 12 + j] += J[k * 12 + i] * J[k * 12 + j];
				}
			}
			ZQ_DImage<double> U_mat(12, 12), S_mat(12, 12), V_mat(12, 12);
			double*& U = U_mat.data();
			double*& S = S_mat.data();
			double*& V = V_mat.data();
			bool suc_flag = ZQ_MathBase::SVD_Decompose(JJ, 12, 12, U, S, V);
			if (!suc_flag)
			{
				return false;
			}

			double RRt[12];
			//memcpy(RRt, V + 11 * 2, sizeof(double) * 12);
			for (int i = 0; i < 12; i++)
			{
				RRt[i] = V[i * 12 + 11];
			}
			double RR[9] =
			{
				RRt[0],RRt[1],RRt[2],
				RRt[4],RRt[5],RRt[6],
				RRt[8],RRt[9],RRt[10]
			};

			//if (ZQ_MathBase::Det(3, RR) < 0)
			if(RRt[11] < 0)
			{
				for (int i = 0; i < 12; i++)
					RRt[i] = -RRt[i];
				for (int i = 0; i < 9; i++)
					RR[i] = -RR[i];
			}

			double Ur[9], Sr[9], Vr[9];
			if (!ZQ_MathBase::SVD_Decompose(RR, 3, 3, Ur, Sr, Vr))
			{
				return false;
			}

			double Rckk[9] = { 0 };
			for (int i = 0; i < 3; i++)
			{
				for (int j = 0; j < 3; j++)
				{
					for (int k = 0; k < 3; k++)
					{
						Rckk[i * 3 + j] += Ur[i * 3 + k] * Vr[j * 3 + k];
					}
				}
			}

			double len_v = 0;
			for (int i = 0; i < 3; i++)
			{
				len_v += RRt[i * 4 + 0] * RRt[i * 4 + 0] + RRt[i * 4 + 1] * RRt[i * 4 + 1] + RRt[i * 4 + 2] * RRt[i * 4 + 2];
			}
			len_v = sqrt(len_v);
			double len_rckk = 0;
			for (int i = 0; i < 9; i++)
				len_rckk += Rckk[i] * Rckk[i];
			len_rckk = sqrt(len_rckk);

			double sc = len_v / len_rckk;
			double inv_sc = (sc == 0) ? (1.0 / (eps*eps)) : (1.0 / sc);
			for (int i = 0; i < 3; i++)
			{
				rT[i + 3] = RRt[i*4+3] * inv_sc;
			}

			if (!zAxis_in)
			{
				Rckk[2] = -Rckk[2];
				Rckk[5] = -Rckk[5];
				Rckk[6] = -Rckk[6];
				Rckk[7] = -Rckk[7];
				rT[5] = -rT[5];
			}
			double rrr[3];
			if (!ZQ_Rodrigues::ZQ_Rodrigues_R2r(Rckk, rrr))
			{
				return false;
			}

			ZQ_Rodrigues::ZQ_Rodrigues_r2R(rrr, Rckk);

			for (int i = 0; i < 3; i++)
				rT[i] = rrr[i];
			//rT[3] = rT[4] = 0;


			if (!use_levmar)
			{
				//// Final optimization(minimize the reprojection error in pixel) : through Gradient Descent :
				ZQ_DImage<float> xn_im_mat(nPts * 2, 1);
				ZQ_DImage<float> dxdrT_im_mat(2 * nPts, 6);
				float*& xn = xn_im_mat.data();
				float*& dxdrT = dxdrT_im_mat.data();

				ZQ_Matrix<double> JJmat(2 * nPts, 6);
				ZQ_Matrix<double> exmat(2 * nPts, 1);
				ZQ_Matrix<double> para(6, 1);
				double* JJ_ptr = JJmat.GetDataPtr();
				double* exmat_ptr = exmat.GetDataPtr();
				double* para_ptr = para.GetDataPtr();
				int iter = 0;
				for (int it = 0; it < max_iter; it++)
				{
					if (!project_points_fun(nPts, X3, rT, xn, zAxis_in)
						|| !project_points_jac(nPts, X3, rT, dxdrT, zAxis_in))
					{
						return false;
					}

					for (int i = 0; i < nPts * 2; i++)
					{
						JJ_ptr[i * 6 + 0] = dxdrT[i * 6 + 0];
						JJ_ptr[i * 6 + 1] = dxdrT[i * 6 + 1];
						JJ_ptr[i * 6 + 2] = dxdrT[i * 6 + 2];
						JJ_ptr[i * 6 + 3] = dxdrT[i * 6 + 3];
						JJ_ptr[i * 6 + 4] = dxdrT[i * 6 + 4];
						JJ_ptr[i * 6 + 5] = dxdrT[i * 6 + 5];
						exmat_ptr[i] = Xn[i] - xn[i];
					}

					if (!ZQ_SVD::Solve(JJmat, para, exmat))
					{
						return false;
					}

					for (int i = 0; i < 6; i++)
					{
						rT[i] += para_ptr[i];
					}
				}
			}
			else
			{
				//// Final optimization(minimize the reprojection error in pixel) : through LevMar :
				
				double x[6] = { rT[0],rT[1],rT[2],rT[3],rT[4],rT[5] };
				ZQ_DImage<double> X3_im(3 * nPts, 1);
				ZQ_DImage<double> fx_im(2 * nPts,1);
				double*& fx = fx_im.data();
				double*& X3_data = X3_im.data();
				for (int i = 0; i < nPts; i++)
				{
					X3_data[i * 3 + 0] = X3[i * 3 + 0];
					X3_data[i * 3 + 1] = X3[i * 3 + 1];
					X3_data[i * 3 + 2] = X3[i * 3 + 2];
					fx[i * 2 + 0] = Xn[i * 2 + 0];
					fx[i * 2 + 1] = Xn[i * 2 + 1];
				}
				double fc_data[2] = { fc[0], fc[1] };
				double cc_data[2] = { cc[0],cc[1] };
				ZQ_LevMarOptions opts;
				ZQ_LevMarReturnInfos info;
				_levmar_data data;
				int m = 6, n = 2 * nPts;
				data.X3 = X3_data;
				data.nPts = nPts;
				data.zAxis_in = zAxis_in;
				opts.init_mu = 0.001;
				if (!ZQ_LevMar::ZQ_LevMar_Der(levmar_fun, levmar_jac, x, fx, m, n, max_iter, opts, info, (void*)(&data), false))
				{
					return false;
				}
				
				for (int i = 0; i < 6; i++)
					rT[i] = x[i];
			}
			return true;
		}

		class _levmar_data
		{
		public:
			const double* X3;
			const double* fc;
			const double* cc;
			int nPts;
			bool zAxis_in;
		};

		static bool levmar_fun(const double* x, double* fx, int m, int n, const void* data)
		{
			const double* rT = x;
			const _levmar_data* arg = (const _levmar_data*)data;
			int nPts = arg->nPts;
			const double* X3 = arg->X3;
			double R[9];
			ZQ_Rodrigues::ZQ_Rodrigues_r2R(rT, R);

			for (int pp = 0; pp < nPts; pp++)
			{
				const double* XX = X3 + pp * 3;
				const double* ttt = rT + 3;
				float Y[3] =
				{
					R[0] * XX[0] + R[1] * XX[1] + R[2] * XX[2] + ttt[0],
					R[3] * XX[0] + R[4] * XX[1] + R[5] * XX[2] + ttt[1],
					R[6] * XX[0] + R[7] * XX[1] + R[8] * XX[2] + ttt[2]
				};

				if (Y[2] == 0)
					return false;

				double inv_Z = 1.0 / Y[2];
				if (!arg->zAxis_in)
					inv_Z = -inv_Z;

				double x2[2] = { Y[0] * inv_Z, Y[1] * inv_Z };

				///////////
				memcpy(fx + pp * 2, x2, sizeof(double) * 2);
			}
			return true;
		}

		static bool levmar_jac(const double* x, double* jx, int m, int n, const void* data)
		{
			/*
			%
			%	[xp,dxpdom,dxpdT,dxpdf,dxpdc,dxpdk, dxpdalpha] = project_points2(X,om,T,f,c,k,alpha)
			%
			%	Projects a 3D structure onto the image plane.
			%
			%	INPUT:
			%		X: 3D structure in the world coordinate frame (3xN matrix for N points)
			%		(r,T): Rigid motion parameters between world coordinate frame and camera reference frame
			%             r: rotation vector (3x1 vector); T: translation vector (3x1 vector)
			%       f: camera focal length in units of horizontal and vertical pixel units (2x1 vector)
			%       c: principal point location in pixel units (2x1 vector)
			%       k: Distortion coefficients (radial and tangential) (5x1 vector)
			%       alpha: Skew coefficient between x and y pixel (alpha = 0 <=> square pixels)
			%
			%	OUTPUT:
			%		xp: Projected pixel coordinates (2xN matrix for N points)
			%       dxpdrT: Derivative of xp with respect to rT ((2N)x6 matrix)
			%       dxpdf: Derivative of xp with respect to f ((2N)x2 matrix if f is 2x1)
			%       dxpdc: Derivative of xp with respect to c ((2N)x2 matrix)
			%       dxpdk: Derivative of xp with respect to k ((2N)x5 matrix)
			%		dxpdalpha: Derivative of xp with respect to alpha ((2N)x1 matrix)
			%
			%	Definitions:
			%		Let P be a point in 3D of coordinates X in the world reference frame (stored in the matrix X)
			%		The coordinate vector of P in the camera reference frame is: Xc = R*X + T
			%		where R is the rotation matrix corresponding to the rotation vector r: R = rodrigues(r);
			%		call x, y and z the 3 coordinates of Xc: x = Xc(1); y = Xc(2); z = Xc(3);
			%		The pinehole projection coordinates of P is [a;b] where a=x/z and b=y/z.
			%		call r^2 = a^2 + b^2.
			%		The distorted point coordinates are: xd = [xx;yy] where:
			%
			%		xx = a * (1 + kc(1)*r^2 + kc(2)*r^4 + kc(5)*r^6)      +      2*kc(3)*a*b + kc(4)*(r^2 + 2*a^2);
			%		yy = b * (1 + kc(1)*r^2 + kc(2)*r^4 + kc(5)*r^6)      +      kc(3)*(r^2 + 2*b^2) + 2*kc(4)*a*b;
			%
			%	The left terms correspond to radial distortion (6th degree), the right terms correspond to tangential distortion
			%
			%	Finally, convertion into pixel coordinates: The final pixel coordinates vector xp=[xxp;yyp] where:
			%
			%	xxp = f(1)*(xx + alpha*yy) + c(1)
			%	yyp = f(2)*yy + c(2)
			%
			%
			%	NOTE: About 90 percent of the code takes care fo computing the Jacobian matrices
			*/


			const double* rT = x;
			const _levmar_data* arg = (const _levmar_data*)data;
			int nPts = arg->nPts;
			const double* X3 = arg->X3;
			
			double R[9], dRdr[27];
			ZQ_Rodrigues::ZQ_Rodrigues_r2R(rT, R, dRdr);

			for (int pp = 0; pp < nPts; pp++)
			{
				const double* XX = X3 + pp * 3;
				const double* ttt = rT + 3;
				double Y[3] =
				{
					R[0] * XX[0] + R[1] * XX[1] + R[2] * XX[2] + ttt[0],
					R[3] * XX[0] + R[4] * XX[1] + R[5] * XX[2] + ttt[1],
					R[6] * XX[0] + R[7] * XX[1] + R[8] * XX[2] + ttt[2]
				};

				if (Y[2] == 0)
					return false;

				double dYdR[27] =
				{
					XX[0], XX[1], XX[2], 0, 0, 0, 0, 0, 0,
					0, 0, 0, XX[0], XX[1], XX[2], 0, 0, 0,
					0, 0, 0, 0, 0, 0, XX[0], XX[1], XX[2]
				};
				double dYdr[9] = { 0 };
				ZQ_MathBase::MatrixMul(dYdR, dRdr, 3, 9, 3, dYdr);
				double dYdT[9] =
				{
					1, 0, 0,
					0, 1, 0,
					0, 0, 1
				};
				double inv_Z = 1.0 / Y[2];
				if (!arg->zAxis_in)
					inv_Z = -inv_Z;
				double x2[2] = { Y[0] * inv_Z, Y[1] * inv_Z };

				double dxdY[6] =
				{
					inv_Z, 0, -x2[0] * inv_Z,
					0, inv_Z, -x2[1] * inv_Z
				};
				if (!arg->zAxis_in)
				{
					dxdY[2] = -dxdY[2];
					dxdY[5] = -dxdY[5];
				}


				double dxdrT[12] = { 0 };
				for (int i = 0; i < 2; i++)
				{
					for (int j = 0; j < 3; j++)
					{
						for (int k = 0; k < 3; k++)
						{
							dxdrT[i * 6 + j] += dxdY[i * 3 + k] * dYdr[k * 3 + j];
							dxdrT[i * 6 + j + 3] += dxdY[i * 3 + k] * dYdT[k * 3 + j];
						}
					}
				}


				///////////
				memcpy(jx + pp * 12, dxdrT, sizeof(double) * 12);
			}

			return true;
		}

		static bool project_points_fun(int nPts, const float* X, const float* rT, float* xp, bool zAxis_in)
		{
			float R[9];
			ZQ_Rodrigues::ZQ_Rodrigues_r2R(rT, R);

			for (int pp = 0; pp < nPts; pp++)
			{
				const float* XX = X + pp * 3;
				const float* ttt = rT + 3;
				float Y[3] =
				{
					R[0] * XX[0] + R[1] * XX[1] + R[2] * XX[2] + ttt[0],
					R[3] * XX[0] + R[4] * XX[1] + R[5] * XX[2] + ttt[1],
					R[6] * XX[0] + R[7] * XX[1] + R[8] * XX[2] + ttt[2]
				};

				if (Y[2] == 0)
					return false;

				float inv_Z = 1.0 / Y[2];
				if (!zAxis_in)
					inv_Z = -inv_Z;

				float x[2] = { Y[0] * inv_Z, Y[1] * inv_Z };


				///////////
				memcpy(xp + pp * 2, x, sizeof(float) * 2);
			}
			return true;
		}

		static bool project_points_jac(int nPts, const float* X, const float* rT, float* dxpdrT, bool zAxis_in)
		{
			/*
			%
			%	[xp,dxpdom,dxpdT,dxpdf,dxpdc,dxpdk, dxpdalpha] = project_points2(X,om,T,f,c,k,alpha)
			%
			%	Projects a 3D structure onto the image plane.
			%
			%	INPUT:
			%		X: 3D structure in the world coordinate frame (3xN matrix for N points)
			%		(r,T): Rigid motion parameters between world coordinate frame and camera reference frame
			%             r: rotation vector (3x1 vector); T: translation vector (3x1 vector)
			%       f: camera focal length in units of horizontal and vertical pixel units (2x1 vector)
			%       c: principal point location in pixel units (2x1 vector)
			%       k: Distortion coefficients (radial and tangential) (5x1 vector)
			%       alpha: Skew coefficient between x and y pixel (alpha = 0 <=> square pixels)
			%
			%	OUTPUT:
			%		xp: Projected pixel coordinates (2xN matrix for N points)
			%       dxpdrT: Derivative of xp with respect to rT ((2N)x6 matrix)
			%       dxpdf: Derivative of xp with respect to f ((2N)x2 matrix if f is 2x1)
			%       dxpdc: Derivative of xp with respect to c ((2N)x2 matrix)
			%       dxpdk: Derivative of xp with respect to k ((2N)x5 matrix)
			%		dxpdalpha: Derivative of xp with respect to alpha ((2N)x1 matrix)
			%
			%	Definitions:
			%		Let P be a point in 3D of coordinates X in the world reference frame (stored in the matrix X)
			%		The coordinate vector of P in the camera reference frame is: Xc = R*X + T
			%		where R is the rotation matrix corresponding to the rotation vector r: R = rodrigues(r);
			%		call x, y and z the 3 coordinates of Xc: x = Xc(1); y = Xc(2); z = Xc(3);
			%		The pinehole projection coordinates of P is [a;b] where a=x/z and b=y/z.
			%		call r^2 = a^2 + b^2.
			%		The distorted point coordinates are: xd = [xx;yy] where:
			%
			%		xx = a * (1 + kc(1)*r^2 + kc(2)*r^4 + kc(5)*r^6)      +      2*kc(3)*a*b + kc(4)*(r^2 + 2*a^2);
			%		yy = b * (1 + kc(1)*r^2 + kc(2)*r^4 + kc(5)*r^6)      +      kc(3)*(r^2 + 2*b^2) + 2*kc(4)*a*b;
			%
			%	The left terms correspond to radial distortion (6th degree), the right terms correspond to tangential distortion
			%
			%	Finally, convertion into pixel coordinates: The final pixel coordinates vector xp=[xxp;yyp] where:
			%
			%	xxp = f(1)*(xx + alpha*yy) + c(1)
			%	yyp = f(2)*yy + c(2)
			%
			%
			%	NOTE: About 90 percent of the code takes care fo computing the Jacobian matrices
			*/

			float R[9], dRdr[27];
			ZQ_Rodrigues::ZQ_Rodrigues_r2R(rT, R, dRdr);

			for (int pp = 0; pp < nPts; pp++)
			{
				const float* XX = X + pp * 3;
				const float* ttt = rT + 3;
				float Y[3] =
				{
					R[0] * XX[0] + R[1] * XX[1] + R[2] * XX[2] + ttt[0],
					R[3] * XX[0] + R[4] * XX[1] + R[5] * XX[2] + ttt[1],
					R[6] * XX[0] + R[7] * XX[1] + R[8] * XX[2] + ttt[2]
				};

				if (Y[2] == 0)
					return false;

				float dYdR[27] =
				{
					XX[0], XX[1], XX[2], 0, 0, 0, 0, 0, 0,
					0, 0, 0, XX[0], XX[1], XX[2], 0, 0, 0,
					0, 0, 0, 0, 0, 0, XX[0], XX[1], XX[2]
				};
				float dYdr[9] = { 0 };
				ZQ_MathBase::MatrixMul(dYdR, dRdr, 3, 9, 3, dYdr);
				float dYdT[9] =
				{
					1, 0, 0,
					0, 1, 0,
					0, 0, 1
				};
				float inv_Z = 1.0 / Y[2];
				if (!zAxis_in)
					inv_Z = -inv_Z;
				float x[2] = { Y[0] * inv_Z, Y[1] * inv_Z };

				float dxdY[6] =
				{
					inv_Z, 0, -x[0] * inv_Z,
					0, inv_Z, -x[1] * inv_Z
				};
				if (!zAxis_in)
				{
					dxdY[2] = -dxdY[2];
					dxdY[5] = -dxdY[5];
				}


				float dxdrT[12] = { 0 };
				for (int i = 0; i < 2; i++)
				{
					for (int j = 0; j < 3; j++)
					{
						for (int k = 0; k < 3; k++)
						{
							dxdrT[i * 6 + j] += dxdY[i * 3 + k] * dYdr[k * 3 + j];
							dxdrT[i * 6 + j + 3] += dxdY[i * 3 + k] * dYdT[k * 3 + j];
						}
					}
				}
				
				///////////
				if (dxpdrT != 0)
					memcpy(dxpdrT + pp * 12, dxdrT, sizeof(float) * 12);
			}

			return true;
		}

		static void EulerAngleFromRotationMatrix(const float* R, float& yaw, float& pitch, float& roll)
		{
			yaw = atan2(-R[6], R[8]);
			pitch = atan2(R[7], sqrt(R[6] * R[6] + R[8] * R[8]));
			roll = atan2(-R[1], R[4]);
		}
	};
	
}
#endif
