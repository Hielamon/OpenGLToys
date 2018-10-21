#pragma once
#include "JoyStick.h"
#include "Scene.h"

namespace SP
{
	//The Camera basic class hold some common properties of the perspective  
	//camera and provide the common camera interface by the virtual function. 
	//The virtual functions(etc. setup, renderOneFrame) can be just called
	//in the inherited class of the GLWindowBase class for the opengl
	//context related implementations in these functions
	class Camera
	{
	public:
		Camera(int width = 0, int height = 0, int offsetX = 0, int offsetY = 0)
			: mCWidth(width), mCHeight(height), mCOffsetX(offsetX), mCOffsetY(offsetY),
			mViewX(offsetX), mViewY(offsetY), mViewWidth(width), mViewHeight(height),
			mbSetup(false), mbClearPerFrame(true), mbDoRender(true), mbShowCanvas(true)
		{
			mpAttachedScene = std::make_shared<Scene>();

			setProjectionMatrix();
			setViewMatrix();
			mJoyStick3D.setTranslateVelocity(CAMERA_MOVE_MIN_SPEED);
			mJoyStick3D.setAngularRate(0.005f);
		}

		~Camera() 
		{
			if (mbSetup)
			{
				//Clear the buffers
				glDeleteBuffers(1, &mViewUBO);
				glDeleteFramebuffers(1, &mMSFBO);
				glDeleteFramebuffers(1, &mFBO);
				glDeleteTextures(1, &mMSColorTexture);
				glDeleteTextures(1, &mColorTexture);
				glDeleteTextures(1, &mMSMeshIDTexture);
				glDeleteTextures(1, &mMeshIDTexture);
				glDeleteRenderbuffers(1, &mMSDepthStencilRBO);
				glDeleteRenderbuffers(1, &mDepthStencilRBO);
			}
		}

		void setClearFlag(bool bClear)
		{
			mbClearPerFrame = bClear;
		}

		void setDoRender(bool bDoRender)
		{
			mbDoRender = bDoRender;
		}

		void setShowCanvas(bool bShowCanvas)
		{
			mbShowCanvas = bShowCanvas;
		}

		bool getShowCanvas()
		{
			return mbShowCanvas;
		}

		void setCanvas(int offsetX, int offsetY, int width, int height)
		{
			mCOffsetX = offsetX;
			mCOffsetY = offsetY;
			mCWidth = width;
			mCHeight = height;
		}

		void getCanvas(int &offsetX, int &offsetY, int &width, int &height)
		{
			offsetX = mCOffsetX;
			offsetY = mCOffsetY;
			width = mCWidth;
			height = mCHeight;
		}

		bool IsValidCanvas()
		{
			if (mCWidth == 0 || mCHeight == 0)
			{
				SP_LOG("One of sides of the camera canvas is zero, camera will not show frame");
			}
			return mCWidth >= 0 && mCHeight >= 0;
		}

		void setViewport(int viewX, int viewY, int viewWidth, int viewHeight)
		{
			mViewX = viewX;
			mViewY = viewY;
			mViewWidth = viewWidth;
			mViewHeight = viewHeight;
		}

		void getViewport(int &viewX, int &viewY, int &viewWidth, int &viewHeight)
		{
			viewX = mViewX;
			viewY = mViewY;
			viewWidth = mViewWidth;
			viewHeight = mViewHeight;
		}

		//set projection matrix
		void setProjectionMatrix(float fovy = glm::radians(60.f), float aspect = 0.0f,
						float zNear = 0.01, float zFar = 100.f)
		{
			mFovy = fovy; mZNear = zNear; mZFar = zFar;
			float winAspect = mCWidth == 0 || mCHeight == 0 ? 1.5f : mCWidth / float(mCHeight);
			mAspect = aspect == 0.0f ? winAspect : aspect;

			mProjMatrix = glm::perspective(mFovy, mAspect, mZNear, mZFar);

			if (mbSetup) _uploadProjectionMatrix();
		}

		glm::mat4 getProjectionMatrix()
		{
			return mProjMatrix;
		}

		void getFrustum(float &fovy, float &aspect, float &zNear, float &zFar)
		{
			fovy = mFovy;
			aspect = mAspect;
			zNear = mZNear;
			zFar = mZFar;
		}

		virtual void setViewMatrix(glm::vec3 &eye = glm::vec3(0.0f, 0.0f, 0.0f),
						   glm::vec3 &center = glm::vec3(0.0f, 0.0f, -1.0f),
						   glm::vec3 &up = glm::vec3(0.0f, 1.0f, 0.0f))
		{
			mEye = eye; mCenter = center; mUp = up;

			mViewMatrix = glm::lookAt(mEye, mCenter, mUp);

			if (mbSetup) _uploadViewMatrix();

			//Synthesize all attached meshes
			if (mpAttachedScene.use_count() != 0)
			{
				std::map<GLuint, std::shared_ptr<Mesh>> &vAttachedMesh =
					mpAttachedScene->getAllMeshes();

				std::for_each(vAttachedMesh.begin(), vAttachedMesh.end(),
							  [&](std::pair<const GLuint, std::shared_ptr<Mesh>>&pair)
				{
					pair.second->setRelMMatrix(glm::inverse(mViewMatrix));
				});
			}
		}
		
		glm::mat4 getViewMatrix()
		{
			return mViewMatrix;
		}

		void getCameraPose(glm::vec3 &eye, glm::vec3 &center, glm::vec3 &up)
		{
			eye = mEye;
			center = mCenter;
			up = mUp;
		}

		JoyStick3D &getJoyStick3D()
		{
			return mJoyStick3D;
		}

		void excuteJoyStick3D(float milisecond)
		{
			/*glm::vec3 eyeTmp = mEye;
			std::cout << "Before eye : " << mEye.x << ", " << mEye.y << ", " << mEye.z << std::endl;
			mJoyStick3D.execute(mEye, mCenter, mUp, milisecond);
			std::cout << "After  eye : " << mEye.x << ", " << mEye.y << ", " << mEye.z << std::endl;
			eyeTmp = mEye - eyeTmp;
			mAcEyeVaring += eyeTmp;
			std::cout << "residual  eye : " << eyeTmp.x << ", " << eyeTmp.y << ", " << eyeTmp.z << std::endl;
			std::cout << "acc eye vary  : " << mAcEyeVaring.x << ", " << mAcEyeVaring.y << ", " << mAcEyeVaring.z << std::endl;
			std::cout << std::endl;
			setViewMatrix(mEye, mCenter, mUp);*/

			mJoyStick3D.execute(mEye, mCenter, mUp, milisecond);
			setViewMatrix(mEye, mCenter, mUp);
		}

		void excuteJoyStick3DTranslate(float milisecond)
		{
			mJoyStick3D.executeTimeTranslate(mEye, mCenter, mUp, milisecond);
			setViewMatrix(mEye, mCenter, mUp);
		}

		void excuteJoyStick3DRotate(float milisecond)
		{
			/*glm::vec3 eyeTmp = mEye;
			std::cout << "Before eye : " << mEye.x << ", " << mEye.y << ", " << mEye.z << std::endl;
			mJoyStick3D.executeTimeRotation(mEye, mCenter, mUp, milisecond);
			std::cout << "After  eye : " << mEye.x << ", " << mEye.y << ", " << mEye.z << std::endl;
			eyeTmp = mEye - eyeTmp;
			mAcEyeVaring += eyeTmp;
			std::cout << "residual  eye : " << eyeTmp.x << ", " << eyeTmp.y << ", " << eyeTmp.z << std::endl;
			std::cout << "acc eye vary  : " << mAcEyeVaring.x << ", " << mAcEyeVaring.y << ", " << mAcEyeVaring.z << std::endl;
			std::cout << std::endl;
			setViewMatrix(mEye, mCenter, mUp);*/

			mJoyStick3D.executeTimeRotation(mEye, mCenter, mUp, milisecond);
			setViewMatrix(mEye, mCenter, mUp);
		}

		void setCameraShape(const std::shared_ptr<Mesh> &pCameraShape)
		{
			mpCameraShape = pCameraShape;
			/*glm::mat4 viewMatrixInv = glm::inverse(mViewMatrix);
			mpCameraShape->transformMesh(viewMatrixInv);*/
		}

		std::shared_ptr<Mesh> getCameraShape()
		{
			return mpCameraShape;
		}

		std::shared_ptr<Scene> getAttachedScene()
		{
			return mpAttachedScene;
		}

		virtual void createCameraShape(glm::vec4 color = glm::vec4(1.0f),
							  glm::mat4 scale = glm::mat4(1.0f))
		{
			std::vector<glm::vec3> vertices(5);
			std::vector<GLuint> indices;
			{
				float tanHalfFovy = std::tan(mFovy*0.5);
				float tanHalfFovx = tanHalfFovy * mAspect;

				vertices[0] = glm::vec3(0.0f, 0.0f, 0.0f);
				vertices[1] = glm::vec3(tanHalfFovx, tanHalfFovy, -1.0f);
				vertices[2] = glm::vec3(tanHalfFovx, -tanHalfFovy, -1.0f);
				vertices[3] = glm::vec3(-tanHalfFovx, -tanHalfFovy, -1.0f);
				vertices[4] = glm::vec3(-tanHalfFovx, tanHalfFovy, -1.0f);

				indices =
				{
					0, 1,
					0, 2,
					0, 3,
					0, 4,
					1, 2,
					2, 3,
					3, 4,
					4, 1
				};
			}

			std::shared_ptr<VertexArray> pVA =
				std::make_shared<VertexArray>(vertices, indices, PrimitiveType::LINES);

			pVA->addInstance(scale);

			std::shared_ptr<Material> pMatrial = std::make_shared<Material>(color);
			mpCameraShape = std::make_shared<Mesh>(pVA, pMatrial);
			mpAttachedScene->addMesh(mpCameraShape);
		}

		//According to looking the bonding box , this function adjust the
		//camera pose to the (1, 1, 1) position of the bounding box
		virtual void adjustCameraPose(BBox bbox)
		{
			glm::vec3 minVertex = bbox.getMinVertex();
			glm::vec3 maxVertex = bbox.getMaxVertex();

			//Guarantee that the minimum edge of box will  
			//greater or equal to the half of maximum edge
			//Now change to cube bounding box
			glm::vec3 diagonal = maxVertex - minVertex;
			float maxEdgeLength = std::max(diagonal.x, 
										   std::max(diagonal.y, diagonal.z));
			//float halfMaxEdgeLength = maxEdgeLength * 0.5;
			glm::vec3 centerVertex = (minVertex + maxVertex)*0.5f;
			for (size_t i = 0; i < 3; i++)
			{
				if (diagonal[i] < maxEdgeLength)
				{
					float offset = (maxEdgeLength - diagonal[i]) * 0.5;
					maxVertex[i] += offset;
					minVertex[i] -= offset;
				}
			}
			BBox bbox_(minVertex, maxVertex);

			glm::vec3 zAxis = glm::normalize(maxVertex - minVertex);
			std::vector<glm::vec3> vVertice = bbox_.getBBoxVertices();
			glm::vec3 diagonal25 = vVertice[5] - vVertice[2];
			glm::vec3 xAxis = glm::normalize(glm::cross(zAxis, diagonal25));
			glm::vec3 yAxis = glm::normalize(glm::cross(zAxis, xAxis));

			glm::vec3 halfDiagonal07 = maxVertex - centerVertex;
			float base = std::sqrt(glm::dot(halfDiagonal07, halfDiagonal07));

			float maxDistance = base + mZNear;
			float tanHalfFovy = tan(mFovy*0.5f);
			for (size_t i = 0; i < vVertice.size(); i++)
			{
				glm::vec3 tmpVec = vVertice[i] - centerVertex;
				float yLength = std::abs(glm::dot(tmpVec, yAxis));
				float xLength = std::abs(glm::dot(tmpVec, xAxis));
				float zLength = glm::dot(tmpVec, zAxis);
				float distanceY = zLength + yLength / tanHalfFovy;
				float distanceX = zLength + xLength / (tanHalfFovy*mAspect);
				float distance = std::max(distanceX, distanceY);
				if (distance > maxDistance) maxDistance = distance;
			}

			//Another way
			/*float maxHalfWidth = 0, maxHalfHeight = 0;
			for (size_t i = 0; i < vVertice.size(); i++)
			{
				glm::vec3 tmpVec = vVertice[i] - centerVertex;
				float yLength = std::abs(glm::dot(tmpVec, yAxis));
				float xLength = std::abs(glm::dot(tmpVec, xAxis));
				float zLength = glm::dot(tmpVec, zAxis);
				if (yLength > maxHalfHeight) maxHalfHeight = yLength;
				if (xLength > maxHalfWidth) maxHalfWidth = xLength;
			}

			glm::vec3 halfDiagonal07 = maxVertex - centerVertex;

			float base = std::sqrt(glm::dot(halfDiagonal07, halfDiagonal07));
			float tanHalfFovy = tan(glm::radians(mFovy)*0.5f);
			float distanceY = base + maxHalfHeight / tanHalfFovy;
			float distanceX = base + maxHalfWidth / (tanHalfFovy*mAspect);
			float distance = std::max(distanceX, distanceY);*/

			glm::vec3 eye = centerVertex + zAxis * maxDistance;
			//glm::vec3 eye = maxVertex + zAxis * mZNear;

			setViewMatrix(eye, centerVertex, -yAxis);
		}

		//Preparation for some OpenGL context settings, Can be only 
		//called from inherited class of the GLWindowBase class
		virtual void setup(int winWidth, int winHeight)
		{
			if (!IsValidCanvas())
			{
				SP_CERR("The canvas of the camera is not valid");
				return;
			}

			if (mbSetup) return;

			
			mBWidth = winWidth;
			mBHeight = winHeight;

			//Using uniform buffers
			glGenBuffers(1, &mViewUBO);
			glBindBuffer(GL_UNIFORM_BUFFER, mViewUBO);
			glBufferData(GL_UNIFORM_BUFFER, 144, NULL, GL_DYNAMIC_DRAW);
			glGenBuffers(GL_UNIFORM_BUFFER, 0);

			_uploadProjectionMatrix();
			_uploadViewMatrix();

			//Create MultiSample FBO
			_createMultiSampleFBO();

			//Create RAW FBO
			_createNormalFBO();

			mpAttachedScene->uploadToDevice();
			mbSetup = true;
		}

		//Can be only called from inherited class of the GLWindowBase class
		virtual void renderScene(const std::shared_ptr<Scene> &pScene)
		{
			if (!mbDoRender) return;
			renderSceneArray(std::vector<std::shared_ptr<Scene>>(1, pScene));
		}

		//Can be only called from inherited class of the GLWindowBase class
		virtual void renderSceneArray(const std::vector<std::shared_ptr<Scene>>
									&vpScene)
		{
			if (!mbDoRender) return;

			if (!mbSetup)
			{
				SP_CERR("The current camera has not been uploaded befor rendering");
				return;
			}

			if (vpScene.size() == 0)
			{
				SP_CERR("The current vpScene is empty");
				return;
			}

			//Bind the vbo point
			glBindBufferBase(GL_UNIFORM_BUFFER, VIEWUBO_BINDING_POINT, mViewUBO);
			glViewport(mViewX, mViewY, mViewWidth, mViewHeight);

			glBindFramebuffer(GL_FRAMEBUFFER, mMSFBO);

			if (mbClearPerFrame)
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			//draw the scene
			for (size_t i = 0; i < vpScene.size(); i++)
			{
				const std::shared_ptr<Scene> &pScene = vpScene[i];
				if (pScene.use_count() == 0) continue;
				pScene->filterVisible(mProjMatrix, mViewMatrix, mZNear, mZFar);
				pScene->draw();
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			//Copy the color buffer from mMSFBO to mFBO
			glBindFramebuffer(GL_READ_FRAMEBUFFER, mMSFBO);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFBO);
			glReadBuffer(GL_COLOR_ATTACHMENT0);
			glDrawBuffer(GL_COLOR_ATTACHMENT0);

			glBlitFramebuffer(0, 0, mBWidth, mBHeight,
							  0, 0, mBWidth, mBHeight,
							  GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
							  GL_STENCIL_BUFFER_BIT, GL_NEAREST);

			//Copy the color buffer from mFBO to default FBO
			glBindFramebuffer(GL_READ_FRAMEBUFFER, mFBO);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glReadBuffer(GL_COLOR_ATTACHMENT0);
			glDrawBuffer(GL_BACK_LEFT);

			if (mbShowCanvas)
			{
				glBlitFramebuffer(mCOffsetX, mCOffsetY, mCWidth + mCOffsetX, mCHeight + mCOffsetY,
								  mCOffsetX, mCOffsetY, mCWidth + mCOffsetX, mCHeight + mCOffsetY,
								  GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
								  GL_STENCIL_BUFFER_BIT, GL_NEAREST);
			}
		}

		virtual void readColorBuffer(std::shared_ptr<unsigned char> &pData,
									 int &width, int &height, int &channel)
		{
			width = mBWidth;
			height = mBHeight;
			channel = 3;

			unsigned char *data = new unsigned char[width * height * channel];
			glBindFramebuffer(GL_READ_FRAMEBUFFER, mFBO);
			glReadBuffer(GL_COLOR_ATTACHMENT0);
			glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);

			unsigned char *dataFlip = new unsigned char[width * height * channel];

			int rowWidth = width * channel;
			for (size_t i = 0, c1 = 0, c2 = rowWidth*(height - 1); 
				 i < height; i++, c1 += rowWidth, c2 -= rowWidth)
			{
				memcpy(dataFlip + c1, data + c2, rowWidth);
			}

			std::shared_ptr<unsigned char> pData_(dataFlip, [](unsigned char *d)
			{
				delete[] d;
			});

			pData = pData_;
		}

	protected:
		glm::vec3 mAcEyeVaring;

		//The three pose vectors of the camera with  
		//respect to the world coordinate
		glm::vec3 mEye, mCenter, mUp;
		//The matrix transforming the world coordinates to camera coordinates
		glm::mat4 mViewMatrix;

		//The Mesh for showing the camera shape
		std::shared_ptr<Mesh> mpCameraShape;
		std::shared_ptr<Scene> mpAttachedScene;
		glm::mat4 mCameraShapeMMatrix;

		//The parameters of perspective frustum
		//mFovy: the angle FOV in y direction
		//mAspect : equal to tanHalfFovx / tanHalfFovy
		float mFovy, mAspect, mZNear, mZFar;
		//The matrix transforming the camera coordinates to clipped coordinates
		glm::mat4 mProjMatrix;
		
		//The pixel size and offset(bottom-left) of camera canvas to the parent window
		int mCWidth, mCHeight, mCOffsetX, mCOffsetY;

		//The variables for viewport(bottom-left)
		int mViewX, mViewY, mViewWidth, mViewHeight;

	protected:
		//Indicate whether the camera has been set up
		bool mbSetup;

		//Indicate whether the camera will do the render
		bool mbDoRender;

		//Indicate whether the camera will show the render result on canvas
		bool mbShowCanvas;

		//The UBO for sharing the view matrix and projection matrix between shaders
		GLuint mViewUBO;

		//The multisample FBO mMSFBO for anti-aliasing,
		//the raw FBO mFBO acting as the temporary FBO
		GLuint mMSFBO, mFBO;

		//Samples for Super sample Anti-aliasing
		static const int mNumSamples = 8;

		//The buffer width and buffer height
		int mBWidth, mBHeight;

		//The buffers for the mMSFBO and mFBO
		//The mColorTexture is not used currently
		GLuint mMSColorTexture, mColorTexture;
		GLuint mMSMeshIDTexture, mMeshIDTexture;
		GLuint mMSDepthStencilRBO, mDepthStencilRBO;

	//private:

		////////////////////////////////////////////////////////
		//Some variables for manipulate the camera
		////////////////////////////////////////////////////////
		JoyStick3D mJoyStick3D;

		bool mbClearPerFrame;

	private:
		void _createMultiSampleFBO()
		{
			int bufferW = mBWidth;
			int bufferH = mBHeight;

			glGenFramebuffers(1, &mMSFBO);
			glBindFramebuffer(GL_FRAMEBUFFER, mMSFBO);

			//Create and attach the color, depth 
			//and extra buffers Color buffers

			//MSAA for anti-aliasing
			glGenTextures(1, &mMSColorTexture);
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mMSColorTexture);
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, mNumSamples, GL_RGB,
									bufferW, bufferH, GL_TRUE);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
								   GL_TEXTURE_2D_MULTISAMPLE, mMSColorTexture, 0);

			glGenTextures(1, &mMSMeshIDTexture);
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mMSMeshIDTexture);
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, mNumSamples, GL_R32UI,
									bufferW, bufferH, GL_TRUE);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
								   GL_TEXTURE_2D_MULTISAMPLE, mMSMeshIDTexture, 0);
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

			GLenum buffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
			glDrawBuffers(2, buffers);

			//Depth and stencil renderbuffer
			glGenRenderbuffers(1, &mMSDepthStencilRBO);
			glBindRenderbuffer(GL_RENDERBUFFER, mMSDepthStencilRBO);
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, mNumSamples,
											 GL_DEPTH24_STENCIL8, bufferW, bufferH);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
									  GL_RENDERBUFFER, mMSDepthStencilRBO);

			glBindRenderbuffer(GL_RENDERBUFFER, 0);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			{
				SP_CERR("The MultSamples FrameBuffer is not complete");
				exit(-1);
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		//Create the FBO without the SSAA technique
		void _createNormalFBO()
		{
			int bufferW = mBWidth;
			int bufferH = mBHeight;

			glGenFramebuffers(1, &mFBO);
			glBindFramebuffer(GL_FRAMEBUFFER, mFBO);

			glGenTextures(1, &mColorTexture);
			glBindTexture(GL_TEXTURE_2D, mColorTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, bufferW, bufferH, 0,
						 GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
								   GL_TEXTURE_2D, mColorTexture, 0);

			glGenTextures(1, &mMeshIDTexture);
			glBindTexture(GL_TEXTURE_2D, mMeshIDTexture);
			/*glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, mwidth, mheight, 0, GL_R,
			GL_UNSIGNED_INT, NULL);*/
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, bufferW, bufferH, 0,
						 GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
								   GL_TEXTURE_2D, mMeshIDTexture, 0);

			GLenum buffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
			glDrawBuffers(2, buffers);

			//Depth and stencil renderbuffer
			glGenRenderbuffers(1, &mDepthStencilRBO);
			glBindRenderbuffer(GL_RENDERBUFFER, mDepthStencilRBO);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, bufferW, bufferH);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
			GL_RENDERBUFFER, mDepthStencilRBO);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
				GL_FRAMEBUFFER_COMPLETE)
			{
				SP_CERR("The Raw FrameBuffer is not complete");
				exit(-1);
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		void _uploadViewMatrix()
		{
			glBindBuffer(GL_UNIFORM_BUFFER, mViewUBO);
			glBufferSubData(GL_UNIFORM_BUFFER, 64, 64, glm::value_ptr(mViewMatrix));
			glBufferSubData(GL_UNIFORM_BUFFER, 128, 12, glm::value_ptr(mEye));
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}

		void _uploadProjectionMatrix()
		{
			glBindBuffer(GL_UNIFORM_BUFFER, mViewUBO);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, 64, glm::value_ptr(mProjMatrix));
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}
	};
}
