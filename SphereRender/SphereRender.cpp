#include <SPhoenix/core.h>

using namespace SP;

int main(int argc, char *argv[])
{
	ShaderCodes shaderCodes("SphereRender.vert", "SphereRender.frag");

	Camera cam(640, 480, "SphereRender");

	Scene scene(shaderCodes);

	cam.addScene(scene);
	cam.run();

	return 0;
}