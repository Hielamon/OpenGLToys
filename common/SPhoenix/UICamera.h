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
				drawScene(vpScene[i]);
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

		~UICamera() {}
	};
}
