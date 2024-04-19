#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

#include "GLCore/Core/Application.h"
#include "GLCore/Core/Input.h"
#include "GLCore/Core/KeyCodes.h"
#include "GLCore/Core/MouseButtonCodes.h"
#include "SandboxLayer.h"

using namespace GLCore;

class Sandbox : public Application
{
public:
	Sandbox()
	{
		PushLayer(new SandboxLayer());
	}
};

int main()
{
	std::unique_ptr<Sandbox> app = std::make_unique<Sandbox>();
	app->Run();
}
