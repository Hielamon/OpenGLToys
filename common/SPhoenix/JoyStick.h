#pragma once
#include "Utils.h"

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

		glm::mat4 getJoyStickSpace()
		{
			return mTwj;
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

		void executeRotation(glm::mat4 &Twl, glm::mat3 R)
		{
			glm::mat4 dR = glm::mat4(R);
			Twl *= (mTjw*glm::transpose(dR)*mTwj);
		}

		void executeRotation(glm::mat4 &Twl, float angle)
		{
			glm::mat4 dR = glm::rotate(glm::mat4(1.0f), angle, mRAxis);
			Twl *= (mTjw*glm::transpose(dR)*mTwj);
		}

		void executeRotation(glm::vec3 &eye, glm::vec3 &center,
							 glm::vec3 &up, glm::mat3 R)
		{
			/*glm::mat4 Twc = glm::lookAt(eye, center, up);
			executeRotation(Twc, R);
			eye = glm::vec3(Twc[3]);
			glm::mat3 iR = glm::transpose(glm::mat3(Twc));
			eye = -iR * eye;

			glm::vec3 zAxis = glm::vec3(Twc[0][2], Twc[1][2], Twc[2][2]);
			center = eye - zAxis;

			up = glm::vec3(Twc[0][1], Twc[1][1], Twc[2][1]);*/

			//Reduce the precision problem
			eye = mRwj * eye + mTswj;
			eye = R * eye;
			eye = mRjw * eye + mTsjw;

			center = mRwj * center + mTswj;
			center = R * center;
			center = mRjw * center + mTsjw;

			up = mRjw * R * mRwj * up;
		}

		void executeRotation(glm::vec3 &eye, glm::vec3 &center,
							 glm::vec3 &up, float angle)
		{
			/*glm::mat4 Twc = glm::lookAt(eye, center, up);
			executeRotation(Twc, angle);
			eye = glm::vec3(Twc[3]);
			glm::mat3 iR = glm::transpose(glm::mat3(Twc));
			eye = -iR * eye;

			glm::vec3 zAxis = glm::vec3(Twc[0][2], Twc[1][2], Twc[2][2]);
			center = eye - zAxis;

			up = glm::vec3(Twc[0][1], Twc[1][1], Twc[2][1]);*/

			//Reduce the precision problem
			glm::mat4 dR = glm::rotate(glm::mat4(1.0f), angle, mRAxis);
			glm::mat3 dRmat3 = glm::mat3(dR);

			executeRotation(eye, center, up, dRmat3);
		}

		void executeTimeRotation(glm::vec3 &eye, glm::vec3 &center,
								 glm::vec3 &up, float milisecond)
		{
			executeRotation(eye, center, up, milisecond * mRAngularRate);
		}

		//execute the rotation in the view space. Specially, the eye point will be 
		//freezed, and this funtion will set the joystick space to view space
		void executeTimeRotationViewSpace(glm::vec3 &eye, glm::vec3 &center,
										  glm::vec3 &up, float milisecond)
		{
			setJoyStickSpace(glm::lookAt(eye, center, up));

			glm::mat4 dR = glm::rotate(glm::mat4(1.0f), milisecond * mRAngularRate, mRAxis);
			glm::mat3 dRmat3 = glm::mat3(dR);

			center = mRwj * center + mTswj;
			center = dRmat3 * center;
			center = mRjw * center + mTsjw;

			up = mRjw * dRmat3 * mRwj * up;
		}

		void executeTranslate(glm::mat4 &Twl, float distance)
		{
			glm::vec3 tVec = mTsDirection * distance;
			glm::mat4 dT = glm::translate(glm::mat4(1.0f), tVec);
			Twl *= (mTjw * glm::inverse(dT)* mTwj);
		}

		//Reduce the precision problem
		void executeTranslate(glm::vec3 &eye, glm::vec3 &center,
							  glm::vec3 &up, float distance)
		{
			glm::vec3 tVec = mRjw * mTsDirection * distance;
			eye += tVec;
			center += tVec;
		}

		//Reduce the precision problem
		void executeTimeTranslate(glm::vec3 &eye, glm::vec3 &center,
								  glm::vec3 &up, float milisecond)
		{
			glm::vec3 tVec = mRjw * mTsDirection * mTsVelocity * milisecond;
			eye += tVec;
			center += tVec;
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
}
