#pragma once
#include "Utils.h"
#include "Scene.h"

namespace SP
{
	class JoyStickGlobal
	{
	public:
		
		~JoyStickGlobal() {}

		static JoyStickGlobal &getInstance()
		{
			static JoyStickGlobal joystick;
			return joystick;
		}

		void accelerateAutoMiniSpeed(float acVelocity)
		{
			mAutoMinMoveSpeed += acVelocity;
			if (mAutoMinMoveSpeed < mMinMoveSpeed)
				mAutoMinMoveSpeed = mMinMoveSpeed;
		}

		float mMinMoveSpeed;
		float mMaxMoveSpeed;
		float mAutoMinMoveSpeed;

		float mAcSpeed, mDeSpeed;

	private:
		JoyStickGlobal() 
		{
			mMinMoveSpeed = 0.0005;
			mMaxMoveSpeed = 0.05;
			mAutoMinMoveSpeed = mMinMoveSpeed;

			mAcSpeed = 0.0001;
			mDeSpeed = -0.0002;
		}
	};

#define CAMERA_MOVE_MIN_SPEED JoyStickGlobal::getInstance().mMinMoveSpeed
#define CAMERA_MOVE_AUTO_MIN_SPEED JoyStickGlobal::getInstance().mAutoMinMoveSpeed
#define CAMERA_MOVE_ACSPEED JoyStickGlobal::getInstance().mAcSpeed
#define CAMERA_MOVE_DESPEED JoyStickGlobal::getInstance().mDeSpeed
#define CAMERA_MOVE_MAX_SPEED JoyStickGlobal::getInstance().mMaxMoveSpeed
#define ACCELERATE_AUTO_MIN_SPEED (x_) 

	class JoyStick3D
	{
	public:
		JoyStick3D() : mbDoTranslate(false), mbDoRotate(false),
			mTsDirection(0.0f, 1.0f, 0.0f), mRAxis(0.0f, 1.0f, 0.0f),
			mTsVelocity(0.0f), mRAngularRate(0.0f)
		{
			setJoyStickSpace(glm::mat4(1.0f));
		}

		~JoyStick3D() {}

		bool getDoTranslate()
		{
			return mbDoTranslate;
		}

		bool getDoRotate()
		{
			return mbDoRotate;
		}

		void setDoTranslate(bool bDoTranslate)
		{
			mbDoTranslate = bDoTranslate;
		}

		void setDoRotate(bool bDoRotate)
		{
			mbDoRotate = bDoRotate;
		}

		void setTranslateDir(glm::vec3 tDirection)
		{
			mTsDirection = tDirection;
		}

		void setRotateAxis(glm::vec3 RAxis)
		{
			mRAxis = RAxis;
		}

		void setTranslateVelocity(float tVelocity)
		{
			mTsVelocity = tVelocity;
		}

		void setAngularRate(float RAngularRate)
		{
			mRAngularRate = RAngularRate;
		}

		void accelerateTsVelocity(float acVelocity)
		{
			mTsVelocity += acVelocity;

			if (mTsVelocity < CAMERA_MOVE_AUTO_MIN_SPEED)
				mTsVelocity = CAMERA_MOVE_AUTO_MIN_SPEED;

			if (mTsVelocity < CAMERA_MOVE_MIN_SPEED)
				mTsVelocity = CAMERA_MOVE_MIN_SPEED;

			if (mTsVelocity > CAMERA_MOVE_MAX_SPEED)
				mTsVelocity = CAMERA_MOVE_MAX_SPEED;
		}

		void setJoyStickSpace(const glm::mat4 &Twj)
		{
			mTwj = Twj;
			mRwj = glm::mat3(Twj);
			mTswj = glm::vec3(mTwj[3]);

			mRjw = glm::transpose(mRwj);
			mTsjw = -mRjw*mTswj;
			mTjw = glm::mat4(mRjw);
			mTjw[3] = glm::vec4(mTsjw, 1.0f);

			//glm::mat4 Test = mTwj * mTjw;
		}

		//The Twl is the transform from world space to the local space,
		//Execute the coordinate defined by the Twl
		void execute(glm::mat4 &Twl, float milisecond)
		{
			if (mbDoRotate && mRAngularRate != 0.0f && milisecond > 0.0f)
			{
				//Do rotate to Pose
				float angle = mRAngularRate * milisecond;

				//Get the rotation axis in the local space of Twl
				executeRotation(Twl, angle);
			}

			if (mbDoTranslate && mTsVelocity != 0.0f && milisecond > 0.0f)
			{
				//Do translate to Pose
				float distance = mTsVelocity * milisecond;

				//Get the translate direction in the local space of Twl
				executeTranslate(Twl, distance);
			}
		}

		void execute(glm::vec3 &eye, glm::vec3 &center, 
					 glm::vec3 &up, float milisecond)
		{
			glm::mat4 Twc = glm::lookAt(eye, center, up);
			execute(Twc, milisecond);
			eye = glm::vec3(Twc[3]);
			glm::mat3 iR = glm::transpose(glm::mat3(Twc));
			eye = -iR * eye;

			glm::vec3 zAxis = glm::vec3(Twc[0][2], Twc[1][2], Twc[2][2]);
			center = eye - zAxis;

			up = glm::vec3(Twc[0][1], Twc[1][1], Twc[2][1]);
		}

		void executeRotation(glm::mat4 &Twl, float angle)
		{
			glm::mat4 dR = glm::rotate(glm::mat4(1.0f), angle, mRAxis);
			Twl *= (mTjw*glm::transpose(dR)*mTwj);
		}

		void executeRotation(glm::vec3 &eye, glm::vec3 &center,
							 glm::vec3 &up, float angle)
		{
			glm::mat4 Twc = glm::lookAt(eye, center, up);
			executeRotation(Twc, angle);
			eye = glm::vec3(Twc[3]);
			glm::mat3 iR = glm::transpose(glm::mat3(Twc));
			eye = -iR * eye;

			glm::vec3 zAxis = glm::vec3(Twc[0][2], Twc[1][2], Twc[2][2]);
			center = eye - zAxis;

			up = glm::vec3(Twc[0][1], Twc[1][1], Twc[2][1]);
		}

		void executeRotation(glm::mat4 &Twl, glm::mat3 R)
		{
			glm::mat4 dR = glm::mat4(R);
			Twl *= (mTjw*glm::transpose(dR)*mTwj);
		}

		void executeRotation(glm::vec3 &eye, glm::vec3 &center,
							 glm::vec3 &up, glm::mat3 R)
		{
			glm::mat4 Twc = glm::lookAt(eye, center, up);
			executeRotation(Twc, R);
			eye = glm::vec3(Twc[3]);
			glm::mat3 iR = glm::transpose(glm::mat3(Twc));
			eye = -iR * eye;

			glm::vec3 zAxis = glm::vec3(Twc[0][2], Twc[1][2], Twc[2][2]);
			center = eye - zAxis;

			up = glm::vec3(Twc[0][1], Twc[1][1], Twc[2][1]);
		}

		void executeTranslate(glm::mat4 &Twl, float distance)
		{
			glm::vec3 tVec = mTsDirection * distance;
			glm::mat4 dT = glm::translate(glm::mat4(1.0f), tVec);
			Twl *= (mTjw * glm::inverse(dT)* mTwj);
		}

		void executeTranslate(glm::vec3 &eye, glm::vec3 &center,
							 glm::vec3 &up, float distance)
		{
			glm::mat4 Twc = glm::lookAt(eye, center, up);
			executeTranslate(Twc, distance);
			eye = glm::vec3(Twc[3]);
			glm::mat3 iR = glm::transpose(glm::mat3(Twc));
			eye = -iR * eye;

			glm::vec3 zAxis = glm::vec3(Twc[0][2], Twc[1][2], Twc[2][2]);
			center = eye - zAxis;

			up = glm::vec3(Twc[0][1], Twc[1][1], Twc[2][1]);
		}

	private:
		//The state for whether to do transform
		bool mbDoTranslate, mbDoRotate;

		//The direction for translate and rotation in joystick space
		glm::vec3 mTsDirection, mRAxis;

		//The velocity for per milisecond
		float mTsVelocity, mRAngularRate;

		//The joystick space, Twj transform the 
		//world point to the joystick space point
		glm::mat4 mTwj, mTjw;
		glm::mat3 mRwj, mRjw;
		//Translate vector
		glm::vec3 mTswj, mTsjw;
	};

	//The virtual functions(etc. setup, renderOneFrame) can be just called
	//in the inherited class of the GLWindowBase class for the opengl
	//context related implementations in these functions
	class Camera
	{
	public:
		Camera(int width = 0, int height = 0, int offsetX = 0, int offsetY = 0)
			: mCWidth(width), mCHeight(height), mCOffsetX(offsetX),
			 mCOffsetY(offsetY), mbSetup(false), mViewX(0), mViewY(0),
			 mViewWidth(width), mViewHeight(height)
		{
			setProjectionMatrix();
			setViewMatrix();
			mJoyStick3D.setTranslateVelocity(CAMERA_MOVE_MIN_SPEED);
			mJoyStick3D.setAngularRate(0.005f);

			mAcEyeVaring = glm::vec3(0.0f);
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
				glDeleteTextures(1, &mMSMeshIDTexture);
				glDeleteTextures(1, &mMeshIDTexture);
				glDeleteRenderbuffers(1, &mMSDepthStencilRBO);
				glDeleteRenderbuffers(1, &mDepthStencilRBO);
			}
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
			return mCWidth > 0 && mCHeight > 0;
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
			mAspect = aspect == 0.0f ? mCWidth / float(mCHeight) : aspect;

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

		//According to looking the bonding box , this function adjust the
		//camera pose to the (1, 1, 1) position of the bounding box
		void adjustCameraPose(BBox bbox)
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
			mbSetup = true;

			//Using uniform buffers
			glGenBuffers(1, &mViewUBO);
			glBindBuffer(GL_UNIFORM_BUFFER, mViewUBO);
			glBufferData(GL_UNIFORM_BUFFER, 144, NULL, GL_DYNAMIC_DRAW);
			glGenBuffers(GL_UNIFORM_BUFFER, 0);

			_uploadProjectionMatrix();
			_uploadViewMatrix();

			//Create MultiSample FBO
			_createMultiSampleFBO(winWidth, winWidth);

			//Create RAW FBO
			_createNormalFBO(winWidth, winWidth);
		}

		//Can be only called from inherited class of the GLWindowBase class
		virtual void renderOneFrame(const std::shared_ptr<Scene> &pScene)
		{
			if(!mbSetup)
			{
				SP_CERR("The current scen has not been uploaded befor drawing");
				return;
			}

			//Bind the vbo point
			glBindBufferBase(GL_UNIFORM_BUFFER, VIEWUBO_BINDING_POINT, mViewUBO);
			glViewport(mViewX, mViewY, mViewWidth, mViewHeight);

			glBindFramebuffer(GL_FRAMEBUFFER, mMSFBO);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			//draw the scene
			drawScene(pScene);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			//Copy the color buffer from mMSFBO to mFBO
			glBindFramebuffer(GL_READ_FRAMEBUFFER, mMSFBO);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glReadBuffer(GL_COLOR_ATTACHMENT0);
			glDrawBuffer(GL_BACK_LEFT);

			glBlitFramebuffer(0, 0, mCWidth, mCHeight, mCOffsetX, mCOffsetY,
							  mCWidth + mCOffsetX, mCHeight + mCOffsetY,
							  GL_COLOR_BUFFER_BIT, GL_NEAREST);
		}

		//Can be only called from inherited class of the GLWindowBase class
		virtual void renderOneFrame(const std::vector<std::shared_ptr<Scene>>
									&vpScene)
		{
			if (!mbSetup)
			{
				SP_CERR("The current scen has not been uploaded befor drawing");
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
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			//draw the scene
			for (size_t i = 0; i < vpScene.size(); i++)
			{
				drawScene(vpScene[i]);
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			//Copy the color buffer from mMSFBO to mFBO
			glBindFramebuffer(GL_READ_FRAMEBUFFER, mMSFBO);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glReadBuffer(GL_COLOR_ATTACHMENT0);
			glDrawBuffer(GL_BACK_LEFT);

			glBlitFramebuffer(0, 0, mCWidth, mCHeight, mCOffsetX, mCOffsetY,
							  mCWidth + mCOffsetX, mCHeight + mCOffsetY,
							  GL_COLOR_BUFFER_BIT, GL_NEAREST);
		}

		virtual std::shared_ptr<Scene> getCameraShapeScene()
		{
			return nullptr;
		}

	protected:
		//The three pose vectors of the camera with  
		//respect to the world coordinate
		glm::vec3 mEye, mCenter, mUp;
		//The matrix transforming the world coordinates to camera coordinates
		glm::mat4 mViewMatrix;

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

		virtual void drawScene(const std::shared_ptr<Scene> &pScene)
		{
			if (pScene.use_count() == 0) return;
			pScene->draw();
		}

	private:
		//Indicatint whether the camera has been set up
		bool mbSetup;

		//The UBO for sharing the view matrix and projection matrix between shaders
		GLuint mViewUBO;

		//The multisample FBO mMSFBO for anti-aliasing,
		//the raw FBO mFBO acting as the temporary FBO
		GLuint mMSFBO, mFBO;

		//The buffers for the FBO
		GLuint mMSColorTexture, mMSMeshIDTexture, mMeshIDTexture;
		GLuint mMSDepthStencilRBO, mDepthStencilRBO;

		//Samples for Super sample Anti-aliasing
		static const int mNumSamples = 8;

		////////////////////////////////////////////////////////
		//Some variables for manipulate the camera
		////////////////////////////////////////////////////////
		JoyStick3D mJoyStick3D;
		glm::vec3 mAcEyeVaring;


	private:
		void _createMultiSampleFBO(int winWidth, int winHeight)
		{
			int bufferW = winWidth;
			int bufferH = winHeight;

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
		void _createNormalFBO(int winWidth, int winHeight)
		{
			int bufferW = winWidth;
			int bufferH = winHeight;

			glGenFramebuffers(1, &mFBO);
			glBindFramebuffer(GL_FRAMEBUFFER, mFBO);

			glGenTextures(1, &mMeshIDTexture);
			glBindTexture(GL_TEXTURE_2D, mMeshIDTexture);
			/*glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, mwidth, mheight, 0, GL_R,
			GL_UNSIGNED_INT, NULL);*/
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, bufferW, bufferH, 0,
						 GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
								   GL_TEXTURE_2D, mMeshIDTexture, 0);

			GLenum buffers[1] = { GL_COLOR_ATTACHMENT0 };
			glDrawBuffers(1, buffers);

			//Depth and stencil renderbuffer
			/*glGenRenderbuffers(1, &mDepthStencilRBO);
			glBindRenderbuffer(GL_RENDERBUFFER, mDepthStencilRBO);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, mwidth, mheight);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
			GL_RENDERBUFFER, mDepthStencilRBO);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);*/

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

	//Add the camera border for distinction between the mini camera and major camera
	class CameraMini : public Camera
	{
	public:
		CameraMini(int width = 0, int height = 0, int offsetX = 0, int offsetY = 0)
			: Camera(width, height, offsetX, offsetY)
		{
			//Setup the border scene
			{
				mpBorderScene = std::make_shared<Scene>();
				std::string __currentPATH = __FILE__;
				__currentPATH = __currentPATH.substr(0, __currentPATH.find_last_of("/\\"));
				ShaderProgram defaultShader(__currentPATH + "/Shaders/SPhoenixScene-2DGraphic.vert",
											__currentPATH + "/Shaders/SPhoenixScene-2DGraphic.frag");
				mpBorderScene->setCommonShaderProgram(defaultShader);

				std::vector<glm::vec3> vertices(4);
				std::vector<GLuint> indices;
				{
					vertices[0] = glm::vec3(1.0f, 1.0f, -1.0f);
					vertices[1] = glm::vec3(1.0f, -1.0f, -1.0f);
					vertices[2] = glm::vec3(-1.0f, -1.0f, -1.0f);
					vertices[3] = glm::vec3(-1.0f, 1.0f, -1.0f);

					indices =
					{
						0, 1, 
						1, 2,
						2, 3,
						3, 0
					};
				}
				std::shared_ptr<VertexArray> pVA = 
					std::make_shared<VertexArray>(vertices, indices, PrimitiveType::LINES);
				pVA->addInstance();

				std::shared_ptr<Material> pMatrial = std::make_shared<Material>();
				std::shared_ptr<Mesh> pBorderMesh = std::make_shared<Mesh>(pVA, pMatrial);
				mpBorderScene->addMesh(pBorderMesh);
			}

			//Setup the camera shape scene
			{
				mpCameraShapeScene = std::make_shared<Scene>();

				std::string __currentPATH = __FILE__;
				__currentPATH = __currentPATH.substr(0, __currentPATH.find_last_of("/\\"));
				ShaderProgram defaultShader(__currentPATH + "/Shaders/SPhoenixScene.vert",
											__currentPATH + "/Shaders/SPhoenixScene.frag");
				mpCameraShapeScene->setCommonShaderProgram(defaultShader);

				std::vector<glm::vec3> vertices(5);
				std::vector<GLuint> indices;
				{
					float tanHalfFovy = std::tan(mFovy*0.5);
					float tanHalfFovx = tanHalfFovy * mAspect;

					float shift = 0.0f/*-mZNear*/;
					vertices[0] = glm::vec3(0.0f, 0.0f, shift);
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
				pVA->addInstance();

				glm::vec4 cameraColor(1.0f, 1.0f, 0.0f, 1.0f);
				std::shared_ptr<Material> pMatrial = std::make_shared<Material>(cameraColor);
				mpCameraShape = std::make_shared<Mesh>(pVA, pMatrial);
				mpCameraShapeScene->addMesh(mpCameraShape);
			}

		}

		virtual std::shared_ptr<Scene> getCameraShapeScene()
		{
			return mpCameraShapeScene;
		}

		virtual void setup(int winWidth, int winHeight)
		{
			Camera::setup(winWidth, winHeight);
			mpBorderScene->uploadToDevice();
			mpCameraShapeScene->uploadToDevice();
		}

		virtual void setViewMatrix(glm::vec3 &eye = glm::vec3(0.0f, 0.0f, 0.0f),
								   glm::vec3 &center = glm::vec3(0.0f, 0.0f, -1.0f),
								   glm::vec3 &up = glm::vec3(0.0f, 1.0f, 0.0f))
		{
			Camera::setViewMatrix(eye, center, up);
			
			//update the camera shape model matrix
			glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f * mZNear * 10.f));
			mpCameraShape->setInstanceMMatrix(glm::inverse(mViewMatrix) * scale, 0);
		}

	protected:
		std::shared_ptr<Scene> mpBorderScene;
		std::shared_ptr<Scene> mpCameraShapeScene;
		std::shared_ptr<Mesh> mpCameraShape;

		virtual void drawScene(const std::shared_ptr<Scene> &pScene)
		{
			Camera::drawScene(pScene);
			mpBorderScene->draw();
		}
	};
}
