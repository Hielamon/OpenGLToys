#pragma once
#include "Camera.h"
#include "Plane.h"

namespace SP
{
	//Omnidirectional camera
	class OmniCamera : public Camera
	{
	public:
		enum ViewPlaneType
		{
			UV_PLANE, CUBE_PLANE, ICO_PLANE
		};

		//Cube unwarpping for showing the Omnidirectional six cube faces
		OmniCamera(int faceSide = 50, int faceTexSide = 1080,
				   int offsetX = 0, int offsetY = 0,
				   ViewPlaneType planeType = UV_PLANE)
			: Camera(faceSide * _computeWidthFactor(planeType), faceSide, offsetX, offsetY),
			mFaceSide(faceSide), mFaceTexSide(faceTexSide), mPlaneType(planeType)
		{
			mpPlaneScene = std::make_shared<Scene>();
			mpMaterialCubeFBO = std::make_shared<MaterialCubeFBO>();

			switch (mPlaneType)
			{
			case SP::OmniCamera::UV_PLANE:
			{
				std::shared_ptr<Plane> pPlane =
					std::make_shared<Plane>(2.0f, 2.0f);
				pPlane->setTexCoord(0.0f, 0.0f, 1.0f, 1.0f);
				pPlane->setMaterial(mpMaterialCubeFBO);

				std::string __currentPATH = __FILE__;
				__currentPATH = __currentPATH.substr(0, __currentPATH.find_last_of("/\\"));

				std::string vertShaderFileOmni = "/Shaders/SPhoenixScene-OmniCamera.vert";
				std::string fragShaderFileOmni = "/Shaders/SPhoenixScene-OmniCamera.frag";
				std::shared_ptr<ShaderProgram> pShaderOmni
					= std::make_shared<ShaderProgram>(__currentPATH + vertShaderFileOmni,
													  __currentPATH + fragShaderFileOmni);

				mpPlaneScene->addMesh(pPlane, pShaderOmni);
			}
				break;
			case SP::OmniCamera::CUBE_PLANE:
			{
				float yInterval = 2.0f / 3.0f, xInterval = 2.0f / 4.0f;
				std::vector<glm::vec3> vVertice(14);
				{
					int vertIdx = 0;
					for (int i = 0; i < 4; i++)
					{
						int jStart = 0, jEnd = 5;
						if (i == 0 || i == 3)
						{
							jStart = 1;
							jEnd = 3;
						}
						for (int j = jStart; j < jEnd; j++)
						{
							vVertice[vertIdx] = glm::vec3(j * xInterval - 1.0f,
														  1.0f - i * yInterval,
														  0.0f);
							vertIdx++;
						}
					}
				}
				std::vector<glm::vec3> vNormal(14);
				{
					vNormal[0] = glm::vec3(-1.0f, 1.0f, 1.0f);
					vNormal[1] = glm::vec3(1.0f, 1.0f, 1.0f);

					vNormal[2] = glm::vec3(-1.0f, 1.0f, 1.0f);
					vNormal[3] = glm::vec3(-1.0f, 1.0f, -1.0f);
					vNormal[4] = glm::vec3(1.0f, 1.0f, -1.0f);
					vNormal[5] = glm::vec3(1.0f, 1.0f, 1.0f);
					vNormal[6] = glm::vec3(-1.0f, 1.0f, 1.0f);

					vNormal[7] = glm::vec3(-1.0f, -1.0f, 1.0f);
					vNormal[8] = glm::vec3(-1.0f, -1.0f, -1.0f);
					vNormal[9] = glm::vec3(1.0f, -1.0f, -1.0f);
					vNormal[10] = glm::vec3(1.0f, -1.0f, 1.0f);
					vNormal[11] = glm::vec3(-1.0f, -1.0f, 1.0f);

					vNormal[12] = glm::vec3(-1.0f, -1.0f, 1.0f);
					vNormal[13] = glm::vec3(1.0f, -1.0f, 1.0f);
				}

				/*(6 * 2 * 3);*/
				std::vector<GLuint> vIndice =
				{
					0, 3, 1, 1, 3, 4,

					2, 7, 3, 3, 7, 8,
					3, 8, 4, 4, 8, 9,
					4, 9, 5, 5, 9, 10,
					5, 10, 6, 6, 10, 11,

					8, 12, 9, 9, 12, 13
				};

				std::shared_ptr<SP::VertexArray> pVA
					= std::make_shared<SP::VertexArray>(vVertice, vIndice);
				pVA->addInstance();
				pVA->setNormals(vNormal);
				std::shared_ptr<Mesh> pMesh = std::make_shared<SP::Mesh>(pVA);
				pMesh->setMaterial(mpMaterialCubeFBO);

				std::string __currentPATH = __FILE__;
				__currentPATH = __currentPATH.substr(0, __currentPATH.find_last_of("/\\"));

				std::string vertShaderFileOmni = "/Shaders/SPhoenixScene-OmniCameraCube.vert";
				std::string fragShaderFileOmni = "/Shaders/SPhoenixScene-OmniCameraCube.frag";
				std::shared_ptr<ShaderProgram> pShaderOmni
					= std::make_shared<ShaderProgram>(__currentPATH + vertShaderFileOmni,
													  __currentPATH + fragShaderFileOmni);

				mpPlaneScene->addMesh(pMesh, pShaderOmni);
			}
				break;
			case SP::OmniCamera::ICO_PLANE:
				break;
			default:
				break;
			}


			//Set the projection matrix for the cube faces
			mCubeProjMatrix = glm::perspective(glm::radians(90.0f), 1.0f, mZNear, mZFar);

			//Set the view matrixs for the cube faces
			mvRelCubeViewMatrix.resize(6);
			mvRelCubeEye.resize(6);
			mvRelCubeCenter.resize(6);
			mvRelCubeUp.resize(6);

			mvCubeViewMatrix.resize(6);
			mvCubeEye.resize(6);
			mvCubeCenter.resize(6);
			mvCubeUp.resize(6);
			//Set the relative cube face view matrixs
			{
				//[0]: GL_TEXTURE_CUBE_MAP_POSITIVE_X	Right
				mvRelCubeEye[0] = glm::vec3(0.0f, 0.0f, 0.0f);
				mvRelCubeCenter[0] = glm::vec3(1.0f, 0.0f, 0.0f);
				mvRelCubeUp[0] = glm::vec3(0.0f, -1.0f, 0.0f);

				//[1]: GL_TEXTURE_CUBE_MAP_NEGATIVE_X	Left
				mvRelCubeEye[1] = glm::vec3(0.0f, 0.0f, 0.0f);
				mvRelCubeCenter[1] = glm::vec3(-1.0f, 0.0f, 0.0f);
				mvRelCubeUp[1] = glm::vec3(0.0f, -1.0f, 0.0f);

				//[2]: GL_TEXTURE_CUBE_MAP_POSITIVE_Y	Top
				mvRelCubeEye[2] = glm::vec3(0.0f, 0.0f, 0.0f);
				mvRelCubeCenter[2] = glm::vec3(0.0f, 1.0f, 0.0f);
				mvRelCubeUp[2] = glm::vec3(0.0f, 0.0f, 1.0f);

				//[3]: GL_TEXTURE_CUBE_MAP_NEGATIVE_Y	Bottom
				mvRelCubeEye[3] = glm::vec3(0.0f, 0.0f, 0.0f);
				mvRelCubeCenter[3] = glm::vec3(0.0f, -1.0f, 0.0f);
				mvRelCubeUp[3] = glm::vec3(0.0f, 0.0f, -1.0f);

				//[4]: GL_TEXTURE_CUBE_MAP_POSITIVE_Z	Back
				mvRelCubeEye[4] = glm::vec3(0.0f, 0.0f, 0.0f);
				mvRelCubeCenter[4] = glm::vec3(0.0f, 0.0f, 1.0f);
				mvRelCubeUp[4] = glm::vec3(0.0f, -1.0f, 0.0f);

				//[5]: GL_TEXTURE_CUBE_MAP_NEGATIVE_Z	Front
				mvRelCubeEye[5] = glm::vec3(0.0f, 0.0f, 0.0f);
				mvRelCubeCenter[5] = glm::vec3(0.0f, 0.0f, -1.0f);
				mvRelCubeUp[5] = glm::vec3(0.0f, -1.0f, 0.0f);

				for (size_t i = 0; i < 6; i++)
				{
					mvRelCubeViewMatrix[i] = glm::lookAt(mvRelCubeEye[i],
														 mvRelCubeCenter[i],
														 mvRelCubeUp[i]);
				}

			}

			//Must after the mvCubeViewMatrix.resize(6) and mvRelCubeViewMatrix.resize(6) 
			setViewMatrix();
		}

		~OmniCamera() {}

		std::shared_ptr<MaterialCubeFBO> getMaterialCubeFBO()
		{
			return mpMaterialCubeFBO;
		}

		virtual void setCanvas(int offsetX, int offsetY, int width, int height)
		{
			int centerX = offsetX + 0.5*width;
			int centerY = offsetY + 0.5*width;

			float widthFactor = _computeWidthFactor(mPlaneType);
			if (height * widthFactor > width)
			{
				int height_ = height;
				height = width * (1.0f / widthFactor);

				offsetY += (height_ - height)*0.5f;
			}
			else if(height * widthFactor < width)
			{
				int width_ = width;
				width = height * widthFactor;

				offsetX += (width_ - width)*0.5f;
			}

			mCOffsetX = offsetX;
			mCOffsetY = offsetY;
			mCWidth = width;
			mCHeight = height;
		}

		virtual void setViewport(int viewX, int viewY, int viewWidth, int viewHeight)
		{
			int centerX = viewX + 0.5*viewWidth;
			int centerY = viewY + 0.5*viewHeight;

			float widthFactor = _computeWidthFactor(mPlaneType);
			if (viewHeight * widthFactor > viewWidth)
			{
				int viewHeight_ = viewHeight;
				viewHeight = viewWidth * (1.0f / widthFactor);

				viewY += (viewHeight_ - viewHeight)*0.5f;
			}
			else if (viewHeight * widthFactor < viewWidth)
			{
				int viewWidth_ = viewWidth;
				viewWidth = viewHeight * widthFactor;

				viewX += (viewWidth_ - viewWidth)*0.5f;
			}

			mViewX = viewX;
			mViewY = viewY;
			mViewWidth = viewWidth;
			mViewHeight = viewHeight;
		}

		virtual void setViewMatrix(glm::vec3 &eye = glm::vec3(0.0f, 0.0f, 0.0f),
								   glm::vec3 &center = glm::vec3(0.0f, 0.0f, -1.0f),
								   glm::vec3 &up = glm::vec3(0.0f, 1.0f, 0.0f))
		{
			Camera::setViewMatrix(eye, center, up);

			//update the cube camera view matrix
			glm::mat3 RInv = glm::transpose(glm::mat3(mViewMatrix));
			glm::vec3 tInv = RInv * (-glm::vec3(mViewMatrix[3]));
			for (size_t i = 0; i < 6; i++)
			{
				mvCubeViewMatrix[i] = mvRelCubeViewMatrix[i] * mViewMatrix;
				mvCubeEye[i] = RInv * mvRelCubeEye[i] + tInv;
				mvCubeCenter[i] = RInv * mvRelCubeCenter[i] + tInv;
				mvCubeUp[i] = RInv * mvRelCubeUp[i];
			}

			if (mbSetup)
			{
				_uploadCubeViewMatrixs();
			}
		}

		virtual void setup(int winWidth, int winHeight)
		{
			if (!IsValidCanvas())
			{
				SP_CERR("The canvas of the camera is not valid");
				return;
			}

			if (mbSetup) return;

			//Using uniform buffers
			mvCubeViewUBO.resize(6);
			glGenBuffers(6, &(mvCubeViewUBO[0]));
			for (size_t i = 0; i < 6; i++)
			{
				glBindBuffer(GL_UNIFORM_BUFFER, mvCubeViewUBO[i]);
				glBufferData(GL_UNIFORM_BUFFER, 144, NULL, GL_DYNAMIC_DRAW);
				glGenBuffers(GL_UNIFORM_BUFFER, 0);
			}

			_uploadCubeProjectionMatrixs();
			_uploadCubeViewMatrixs();

			_createCubeMultiSampleFBO();
			_createCubeNormalFBO();

			Camera::setup(winWidth, winHeight);


			mpMaterialCubeFBO->setCubeTextureBuffer1(mCubeColorTexture);

			mpPlaneScene->uploadToDevice();
		}

		//Can be only called from inherited class of the GLWindowBase class
		virtual void renderSceneArray(const std::vector<std::shared_ptr<Scene>>
									  &vpScene)
		{
			if (!mbDoRender) return;

			//return;
			if (!mbSetup)
			{
				SP_CERR("The current camera has not been uploaded befor rendering");
				return;
			}

			if (vpScene.size() == 0)
			{
				SP_CERR("The current vpScene for OmniCamera rendering is empty");
				return;
			}

			/*float scale1 = mCWidth / (4 * mFaceSide);
			float scale2 = mCHeight / (3 * mFaceSide);
			float scale = std::min(scale1, scale2);
			float faceSideScaled = mFaceSide * scale;
			glm::vec2 dOffset(0.5 * (mCWidth - faceSideScaled * 4),
							  0.5 * (mCHeight - faceSideScaled * 3));
			std::vector<glm::vec4> vRect =
			{
				{ 2, 1, 1, 1 },
				{ 0, 1, 1, 1 },
				{ 1, 2, 1, 1 },
				{ 1, 0, 1, 1 },
				{ 3, 1, 1, 1 },
				{ 1, 1, 1, 1 }
			};*/

			for (size_t i = 0; i < 6; i++)
			{
				glBindFramebuffer(GL_FRAMEBUFFER, mCubeMSFBO);
				glViewport(0, 0, mFaceTexSide, mFaceTexSide);

				//Bind the vbo point
				glBindBufferBase(GL_UNIFORM_BUFFER, VIEWUBO_BINDING_POINT, mvCubeViewUBO[i]);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

				//draw the scenee
				for (size_t j = 0; j < vpScene.size(); j++)
				{
					const std::shared_ptr<Scene> &pScene = vpScene[j];

					if (pScene.use_count() == 0) continue;
					pScene->filterVisible(mCubeProjMatrix, mvCubeViewMatrix[i], mZNear, mZFar);
					pScene->draw();
				}

				//Copy the buffers from mCubeMSFBO to mvCubeFBO[i]
				glBindFramebuffer(GL_READ_FRAMEBUFFER, mCubeMSFBO);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mvCubeFBO[i]);
				glReadBuffer(GL_COLOR_ATTACHMENT0);
				glDrawBuffer(GL_COLOR_ATTACHMENT0);

				glBlitFramebuffer(0, 0, mFaceTexSide, mFaceTexSide,
								  0, 0, mFaceTexSide, mFaceTexSide,
								  GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
								  GL_STENCIL_BUFFER_BIT, GL_NEAREST);

				
			}

			if (mpPlaneScene.use_count() != 0)
			{
				glBindBufferBase(GL_UNIFORM_BUFFER, VIEWUBO_BINDING_POINT, mViewUBO);
				//glViewport(0, 0, mBWidth, mBHeight);
				glViewport(mViewX, mViewY, mViewWidth, mViewHeight);

				glBindFramebuffer(GL_FRAMEBUFFER, mMSFBO);
				if (mbClearPerFrame)
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

				mpPlaneScene->draw();

				//Copy the color buffer from mMSFBO to mFBO
				glBindFramebuffer(GL_READ_FRAMEBUFFER, mMSFBO);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFBO);
				glReadBuffer(GL_COLOR_ATTACHMENT0);
				glDrawBuffer(GL_COLOR_ATTACHMENT0);

				glBlitFramebuffer(0, 0, mBWidth, mBHeight,
								  0, 0, mBWidth, mBHeight,
								  GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
								  GL_STENCIL_BUFFER_BIT, GL_NEAREST);

				if (mbShowCanvas)
				{
					//Copy the color buffer from mFBO to default FBO
					glBindFramebuffer(GL_READ_FRAMEBUFFER, mFBO);
					glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
					glReadBuffer(GL_COLOR_ATTACHMENT0);
					glDrawBuffer(GL_BACK_LEFT);

					glBlitFramebuffer(mCOffsetX, mCOffsetY, mCWidth + mCOffsetX, mCHeight + mCOffsetY,
									  mCOffsetX, mCOffsetY, mCWidth + mCOffsetX, mCHeight + mCOffsetY,
									  GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
									  GL_STENCIL_BUFFER_BIT, GL_NEAREST);
				}
			}
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

	protected:
		//The face side length in the window cube unwarped canvas
		int mFaceSide;

		//The face side length for the cube texture
		int mFaceTexSide;

		std::vector<GLuint> mvCubeViewUBO;

		std::shared_ptr<Scene> mpPlaneScene;
		std::shared_ptr<MaterialCubeFBO> mpMaterialCubeFBO;
		ViewPlaneType mPlaneType;
		

		//The FBOs for six cube faces:
		//[0]: GL_TEXTURE_CUBE_MAP_POSITIVE_X	Right
		//[1]: GL_TEXTURE_CUBE_MAP_NEGATIVE_X	Left
		//[2]: GL_TEXTURE_CUBE_MAP_POSITIVE_Y	Top
		//[3]: GL_TEXTURE_CUBE_MAP_NEGATIVE_Y	Bottom
		//[4]: GL_TEXTURE_CUBE_MAP_POSITIVE_Z	Back
		//[5]: GL_TEXTURE_CUBE_MAP_NEGATIVE_Z	Front
		std::vector<GLuint> mvCubeFBO;
		//std::vector<GLuint> mvCubeColorTexture;
		GLuint mCubeColorTexture;
		std::vector<GLuint> mvCubeDepthStencilRBO;

		//The multisample FBO mMSFBO for anti - aliasing
		GLuint mCubeMSFBO, mCubeMSColorTexture, mCubeMSDepthStencilRBO;

		//The six relative view matrixs for each face
		std::vector<glm::mat4> mvRelCubeViewMatrix;
		std::vector<glm::vec3> mvRelCubeEye, mvRelCubeCenter, mvRelCubeUp;

		//The six absolute view matrixs for each face
		std::vector<glm::mat4> mvCubeViewMatrix;
		std::vector<glm::vec3> mvCubeEye, mvCubeCenter, mvCubeUp;
		glm::mat4 mCubeProjMatrix;

	private:
		void _createCubeMultiSampleFBO()
		{
			int bufferW = mFaceTexSide;
			int bufferH = mFaceTexSide;

			glGenFramebuffers(1, &mCubeMSFBO);
			glBindFramebuffer(GL_FRAMEBUFFER, mCubeMSFBO);

			//Create and attach the color, depth 
			//and extra buffers Color buffers

			//MSAA for anti-aliasing
			glGenTextures(1, &mCubeMSColorTexture);
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mCubeMSColorTexture);
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, mNumSamples, GL_RGB,
									bufferW, bufferH, GL_TRUE);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
								   GL_TEXTURE_2D_MULTISAMPLE,
								   mCubeMSColorTexture, 0);

			GLenum buffers[1] = { GL_COLOR_ATTACHMENT0 };
			glDrawBuffers(1, buffers);

			//Depth and stencil renderbuffer
			glGenRenderbuffers(1, &mCubeMSDepthStencilRBO);
			glBindRenderbuffer(GL_RENDERBUFFER, mCubeMSDepthStencilRBO);
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, mNumSamples,
											 GL_DEPTH24_STENCIL8, bufferW, bufferH);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
									  GL_RENDERBUFFER, mCubeMSDepthStencilRBO);

			glBindRenderbuffer(GL_RENDERBUFFER, 0);

			//GL_DEBUG_ALL;

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			{
				SP_CERR("The Cube MultSamples FrameBuffer is not complete");
				exit(-1);
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		//Create the FBO without the SSAA technique
		void _createCubeNormalFBO()
		{
			mvCubeFBO.resize(6);
			//mvCubeColorTexture.resize(6);
			mvCubeDepthStencilRBO.resize(6);

			int bufferW = mFaceTexSide;
			int bufferH = mFaceTexSide;

			glGenFramebuffers(6, &(mvCubeFBO[0]));
			//glGenTextures(6, &(mvCubeColorTexture[0]));
			glGenRenderbuffers(6, &(mvCubeDepthStencilRBO[0]));

			{
				glGenTextures(1, &mCubeColorTexture);
				glBindTexture(GL_TEXTURE_CUBE_MAP, mCubeColorTexture);

				for (size_t i = 0; i < 6; i++)
				{
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
								 GL_RGB8, bufferW, bufferH, 0,
								 GL_RGB, GL_UNSIGNED_BYTE, NULL);
				}

				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				//since texture coordinates that are exactly between two faces might 
				//not hit an exact face (due to some hardware limitations) so by using
				// GL_CLAMP_TO_EDGE OpenGL always return their edge values whenever
				// we sample between faces.
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

				glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
				glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
			}


			for (size_t i = 0; i < 6; i++)
			{
				glBindFramebuffer(GL_FRAMEBUFFER, mvCubeFBO[i]);

				/*glBindTexture(GL_TEXTURE_2D, mvCubeColorTexture[i]);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, bufferW, bufferH, 0,
							 GL_RGB, GL_UNSIGNED_BYTE, NULL);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
									   GL_TEXTURE_2D, mvCubeColorTexture[i], 0);*/

				//glBindTexture(GL_TEXTURE_CUBE_MAP, mCubeColorTexture);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
									   GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mCubeColorTexture, 0);
				glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

				GLenum buffers[1] = { GL_COLOR_ATTACHMENT0 };
				glDrawBuffers(1, buffers);

				//Depth and stencil renderbuffer
				glBindRenderbuffer(GL_RENDERBUFFER, mvCubeDepthStencilRBO[i]);
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
									  bufferW, bufferH);
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
										  GL_RENDERBUFFER, mvCubeDepthStencilRBO[i]);
				glBindRenderbuffer(GL_RENDERBUFFER, 0);


				if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
					GL_FRAMEBUFFER_COMPLETE)
				{
					SP_CERR("The Raw FrameBuffer of face[" << i << "] is \
								not complete");
					exit(-1);
				}

				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}
		}

		void _uploadCubeViewMatrixs()
		{
			for (size_t i = 0; i < 6; i++)
			{
				glBindBuffer(GL_UNIFORM_BUFFER, mvCubeViewUBO[i]);
				glBufferSubData(GL_UNIFORM_BUFFER, 64, 64, glm::value_ptr(mvCubeViewMatrix[i]));
				glBufferSubData(GL_UNIFORM_BUFFER, 128, 12, glm::value_ptr(mvCubeEye[i]));
				glBindBuffer(GL_UNIFORM_BUFFER, 0);
			}
		}

		void _uploadCubeProjectionMatrixs()
		{
			for (size_t i = 0; i < 6; i++)
			{
				glBindBuffer(GL_UNIFORM_BUFFER, mvCubeViewUBO[i]);
				glBufferSubData(GL_UNIFORM_BUFFER, 0, 64, glm::value_ptr(mCubeProjMatrix));
				glBindBuffer(GL_UNIFORM_BUFFER, 0);
			}
		}

		static float _computeWidthFactor(ViewPlaneType planeType)
		{
			float result = 1.0f;
			switch (planeType)
			{
			case SP::OmniCamera::UV_PLANE:
				result = 2.0f;
				break;
			case SP::OmniCamera::CUBE_PLANE:
				result = 4.0f / 3.0f;
				break;
			case SP::OmniCamera::ICO_PLANE:
				result = 11.0f / (std::sqrt(5.0f) * 3.0f);
				break;
			default:
				break;
			}
			return result;
		}
	};

	/*class FishEyeCamera : public OmniCamera
	{
	public:
		 
	};*/
}
