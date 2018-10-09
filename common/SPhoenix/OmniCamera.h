#pragma once
#include "Camera.h"

namespace SP
{
	//Omnidirectional camera
	class OmniCamera : public Camera
	{
	public:
		//Cube unwarpping for showing the Omnidirectional six cube faces
		OmniCamera(int faceSide = 50, int faceTexSide = 1080,
				   int offsetX = 0, int offsetY = 0)
			: Camera(faceSide * 4, faceSide * 3, offsetX, offsetY),
			mFaceSide(faceSide), mFaceTexSide(faceTexSide)
		{
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
				mvRelCubeUp[0] = glm::vec3(0.0f, 1.0f, 0.0f);

				//[1]: GL_TEXTURE_CUBE_MAP_NEGATIVE_X	Left
				mvRelCubeEye[1] = glm::vec3(0.0f, 0.0f, 0.0f);
				mvRelCubeCenter[1] = glm::vec3(-1.0f, 0.0f, 0.0f);
				mvRelCubeUp[1] = glm::vec3(0.0f, 1.0f, 0.0f);

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
				mvRelCubeUp[4] = glm::vec3(0.0f, 1.0f, 0.0f);

				//[5]: GL_TEXTURE_CUBE_MAP_NEGATIVE_Z	Front
				mvRelCubeEye[5] = glm::vec3(0.0f, 0.0f, 0.0f);
				mvRelCubeCenter[5] = glm::vec3(0.0f, 0.0f, -1.0f);
				mvRelCubeUp[5] = glm::vec3(0.0f, 1.0f, 0.0f);

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
		}

		//Can be only called from inherited class of the GLWindowBase class
		virtual void renderSceneArray(const std::vector<std::shared_ptr<Scene>>
									  &vpScene)
		{
			//return;
			if (!mbSetup)
			{
				SP_CERR("The current scen has not been uploaded befor drawing");
				return;
			}

			if (vpScene.size() == 0)
			{
				SP_CERR("The current vpScene for OmniCamera rendering is empty");
				return;
			}

			float scale1 = mCWidth / (4 * mFaceSide);
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
			};

			for (size_t i = 0; i < 6; i++)
			{
				glBindFramebuffer(GL_FRAMEBUFFER, mCubeMSFBO);
				glViewport(0, 0, mFaceTexSide, mFaceTexSide);

				//Bind the vbo point
				glBindBufferBase(GL_UNIFORM_BUFFER, VIEWUBO_BINDING_POINT, mvCubeViewUBO[i]);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

				//draw the scene
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

				//Copy the buffers from mvCubeFBO[i] to default buffer
				glBindFramebuffer(GL_READ_FRAMEBUFFER, mvCubeFBO[i]);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
				glReadBuffer(GL_COLOR_ATTACHMENT0);
				glDrawBuffer(GL_BACK_LEFT);

				int winX = vRect[i].x * faceSideScaled + mCOffsetX + dOffset.x,
					winY = vRect[i].y * faceSideScaled + mCOffsetY + dOffset.y,
					winWidth = vRect[i].z * faceSideScaled,
					winHeight = vRect[i].w * faceSideScaled;
				glBlitFramebuffer(0, 0, mFaceTexSide, mFaceTexSide,
								  winX, winY, winWidth + winX, winHeight + winY,
								  GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
								  GL_STENCIL_BUFFER_BIT, GL_NEAREST);
			}
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

	protected:
		//The face side length in the window cube unwarped canvas
		int mFaceSide;

		//The face side length for the cube texture
		int mFaceTexSide;

		std::vector<GLuint> mvCubeViewUBO;

		//The FBOs for six cube faces:
		//[0]: GL_TEXTURE_CUBE_MAP_POSITIVE_X	Right
		//[1]: GL_TEXTURE_CUBE_MAP_NEGATIVE_X	Left
		//[2]: GL_TEXTURE_CUBE_MAP_POSITIVE_Y	Top
		//[3]: GL_TEXTURE_CUBE_MAP_NEGATIVE_Y	Bottom
		//[4]: GL_TEXTURE_CUBE_MAP_POSITIVE_Z	Back
		//[5]: GL_TEXTURE_CUBE_MAP_NEGATIVE_Z	Front
		std::vector<GLuint> mvCubeFBO;
		std::vector<GLuint> mvCubeColorTexture;
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
			mvCubeColorTexture.resize(6);
			mvCubeDepthStencilRBO.resize(6);

			int bufferW = mFaceTexSide;
			int bufferH = mFaceTexSide;

			glGenFramebuffers(6, &(mvCubeFBO[0]));
			glGenTextures(6, &(mvCubeColorTexture[0]));
			glGenRenderbuffers(6, &(mvCubeDepthStencilRBO[0]));

			for (size_t i = 0; i < 6; i++)
			{
				glBindFramebuffer(GL_FRAMEBUFFER, mvCubeFBO[i]);

				glBindTexture(GL_TEXTURE_2D, mvCubeColorTexture[i]);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, bufferW, bufferH, 0,
							 GL_RGB, GL_UNSIGNED_BYTE, NULL);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
									   GL_TEXTURE_2D, mvCubeColorTexture[i], 0);

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
	};

	class FishEyeCamera : public OmniCamera
	{
	public:

	};
}
