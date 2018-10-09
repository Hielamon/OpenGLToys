#pragma once

#include "Camera.h"

namespace SP
{
	//For drawing the 2D UI graphic
	class UICamera : public Camera 
	{
	public:
		UICamera(int width = 0, int height = 0, int offsetX = 0, int offsetY = 0)
			: Camera(width, height, offsetX, offsetY)
		{
			float left = mCOffsetX, right = mCOffsetX + mCWidth;
			float bottom = mCOffsetY, top = mCOffsetY + mCHeight;

			mProjMatrix = glm::ortho(left, right, bottom, top);

			mZNear = -1;
			mZFar = 1;
		}

		//Can be only called from inherited class of the GLWindowBase class
		virtual void renderSceneArray(const std::vector<std::shared_ptr<Scene>>
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

			//Copy pre-rendered results
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mMSFBO);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
			glReadBuffer(GL_BACK_LEFT);
			glDrawBuffer(GL_COLOR_ATTACHMENT0);

			glBlitFramebuffer( mCOffsetX, mCOffsetY,
							  mCWidth + mCOffsetX, mCHeight + mCOffsetY,
							  0, 0, mCWidth, mCHeight,
							  GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
							  GL_STENCIL_BUFFER_BIT, GL_NEAREST);


			glBindFramebuffer(GL_FRAMEBUFFER, mMSFBO);

			//draw the scene
			glDisable(GL_DEPTH_TEST);
			for (size_t i = 0; i < vpScene.size(); i++)
			{
				const std::shared_ptr<Scene> &pScene = vpScene[i];
				if (pScene.use_count() == 0) continue;
				pScene->draw();
			}
			glEnable(GL_DEPTH_TEST);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			//Copy the color buffer from mFBO to default FBO
			glBindFramebuffer(GL_READ_FRAMEBUFFER, mMSFBO);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glReadBuffer(GL_COLOR_ATTACHMENT0);
			glDrawBuffer(GL_BACK_LEFT);

			glBlitFramebuffer(0, 0, mCWidth, mCHeight, mCOffsetX, mCOffsetY,
							  mCWidth + mCOffsetX, mCHeight + mCOffsetY,
							  GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
							  GL_STENCIL_BUFFER_BIT, GL_NEAREST);

		}

		virtual void roughRenderScene(const std::shared_ptr<Scene> &pScene)
		{
			if (!mbSetup)
			{
				SP_CERR("The current scen has not been uploaded befor drawing");
				return;
			}

			//Bind the vbo point
			glBindBufferBase(GL_UNIFORM_BUFFER, VIEWUBO_BINDING_POINT, mViewUBO);
			glViewport(mViewX, mViewY, mViewWidth, mViewHeight);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			//draw the scene
			glDisable(GL_DEPTH_TEST);
			if (pScene.use_count() != 0)
			{
				pScene->draw();
			}
			glEnable(GL_DEPTH_TEST);
		}

		~UICamera() {}
	};
}
