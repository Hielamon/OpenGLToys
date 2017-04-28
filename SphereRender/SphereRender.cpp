#include <SPhoenix/core.h>

using namespace SP;

int main(int argc, char *argv[])
{
	ShaderCodes shaderCodes("SphereRender.vert", "SphereRender.frag");
	Scene scene(shaderCodes);

	Camera cam(640, 480, "SphereRender");

	cam.addScene(scene);
	cam.run();

	return 0;
}