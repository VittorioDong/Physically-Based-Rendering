#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <shader.h>
#include <camera.h>
#include <model.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);
void renderPbrModel(unsigned int albedoMap, unsigned int normalMap, unsigned int metallicMap, unsigned int roughnessMap, unsigned int aoMap, Shader& pbrShader, Model inputModel, glm::mat4 model);
void renderSphere();
void renderCube();
void renderQuad();

// 窗体宽高
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// 实例化相机
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
bool firstMouse = true;

// 计时
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
	// 初始化glfw
	glfwInit();
	// 设置OpenGL的主要和次要版本为3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// 设置多重采样样本数量为4，用于抗锯齿
	glfwWindowHint(GLFW_SAMPLES, 4);
	// 设置OpenGL的配置文件为核心配置文件
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// 创建glfw窗体
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	// 设置当前的上下文为此窗口
	glfwMakeContextCurrent(window);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	// 设置回调函数
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// 设置GLFW捕获鼠标
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// 使用glad加载所有OpenGL函数指针
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// 初始化ImGui上下文和设置风格
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui::StyleColorsDark();
	ImGui_ImplOpenGL3_Init("#version 330 core");
	ImGui_ImplGlfw_InitForOpenGL(window, true);


	// 配置全局OpenGL状态
	glEnable(GL_DEPTH_TEST);
	// 设置深度函数为小于等于，用于天空盒深度技巧
	glDepthFunc(GL_LEQUAL);
	// 启用无缝立方贴图采样，为预滤镜贴图的低mip级别
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	// 构建和编译着色器
	Shader pbrShader("pbr.vs", "pbr.fs");
	Shader equirectangularToCubemapShader("cubemap.vs", "equirectangular_to_cubemap.fs");
	Shader irradianceShader("cubemap.vs", "irradiance_convolution.fs");
	Shader prefilterShader("cubemap.vs", "prefilter.fs");
	Shader brdfShader("brdf.vs", "brdf.fs");
	Shader backgroundShader("background.vs", "background.fs");

	// 配置着色器中的纹理单元
	pbrShader.use();
	pbrShader.setInt("irradianceMap", 0);
	pbrShader.setInt("prefilterMap", 1);
	pbrShader.setInt("brdfLUT", 2);
	pbrShader.setInt("albedoMap", 3);
	pbrShader.setInt("normalMap", 4);
	pbrShader.setInt("metallicMap", 5);
	pbrShader.setInt("roughnessMap", 6);
	pbrShader.setInt("aoMap", 7);

	backgroundShader.use();
	backgroundShader.setInt("environmentMap", 0);

	// 加载PBR材料纹理
	// 黄金材质
	unsigned int goldAlbedoMap = loadTexture("resources/textures/pbr/gold/albedo.png");
	unsigned int goldNormalMap = loadTexture("resources/textures/pbr/gold/normal.png");
	unsigned int goldMetallicMap = loadTexture("resources/textures/pbr/gold/metallic.png");
	unsigned int goldRoughnessMap = loadTexture("resources/textures/pbr/gold/roughness.png");
	unsigned int goldAOMap = loadTexture("resources/textures/pbr/gold/ao.png");

	stbi_set_flip_vertically_on_load(true);
	unsigned int modelAlbedoMap0 = loadTexture("resources/objects/pokeball/albedo.png");
	unsigned int modelNormalMap0 = loadTexture("resources/objects/pokeball/normal.png");
	unsigned int modelMetallicMap0 = loadTexture("resources/objects/pokeball/metallic.png");
	unsigned int modelRoughnessMap0 = loadTexture("resources/objects/pokeball/roughness.png");
	unsigned int modelAoMapModel0 = loadTexture("resources/objects/pokeball/ao.png");
	cout << "loadTexture from " << "resources/objects/pokeball" << endl;

	stbi_set_flip_vertically_on_load(false);
	unsigned int modelAlbedoMap1 = loadTexture("resources/objects/tank/hull_albedo.jpg");
	unsigned int modelNormalMap1 = loadTexture("resources/objects/tank/hull_normal.jpg");
	unsigned int modelMetallicMap1 = loadTexture("resources/objects/tank/hull_metallic.jpg");
	unsigned int modelRoughnessMap1 = loadTexture("resources/objects/tank/hull_roughness.jpg");
	unsigned int modelAoMapModel1 = loadTexture("resources/objects/tank/hull_ao.jpg");

	unsigned int modelAlbedoMap2 = loadTexture("resources/objects/tank/track_albedo.jpg");
	unsigned int modelNormalMap2 = loadTexture("resources/objects/tank/track_normal.jpg");
	unsigned int modelMetallicMap2 = loadTexture("resources/objects/tank/track_metallic.jpg");
	unsigned int modelRoughnessMap2 = loadTexture("resources/objects/tank/track_roughness.jpg");
	unsigned int modelAoMapModel2 = loadTexture("resources/objects/tank/track_ao.jpg");

	unsigned int modelAlbedoMap3 = loadTexture("resources/objects/tank/turret_albedo.jpg");
	unsigned int modelNormalMap3 = loadTexture("resources/objects/tank/turret_normal.jpg");
	unsigned int modelMetallicMap3 = loadTexture("resources/objects/tank/turret_metallic.jpg");
	unsigned int modelRoughnessMap3 = loadTexture("resources/objects/tank/turret_roughness.jpg");
	unsigned int modelAoMapModel3 = loadTexture("resources/objects/tank/turret_ao.jpg");

	unsigned int modelAlbedoMap4 = loadTexture("resources/objects/tank/wheels_albedo.jpg");
	unsigned int modelNormalMap4 = loadTexture("resources/objects/tank/wheels_normal.jpg");
	unsigned int modelMetallicMap4 = loadTexture("resources/objects/tank/wheels_metallic.jpg");
	unsigned int modelRoughnessMap4 = loadTexture("resources/objects/tank/wheels_roughness.jpg");
	unsigned int modelAoMapModel4 = loadTexture("resources/objects/tank/wheels_ao.jpg");

	unsigned int modelAlbedoMap5 = loadTexture("resources/objects/tank/floor_albedo.jpg");
	unsigned int modelNormalMap5 = loadTexture("resources/objects/tank/floor_normal.jpg");
	unsigned int modelMetallicMap5 = loadTexture("resources/objects/tank/floor_metallic.jpg");
	unsigned int modelRoughnessMap5 = loadTexture("resources/objects/tank/floor_roughness.jpg");
	unsigned int modelAoMapModel5 = loadTexture("resources/objects/tank/floor_ao.jpg");
	cout << "loadTexture from " << "resources/objects/tank" << endl;

	// 实例化模型
	Model pokeball("resources/objects/pokeball/PokeBall.obj");
	cout << "init model finish " << "resources/objects/pokeball/PokeBall.obj" << endl;
	Model hull("resources/objects/tank/hull.obj");
	cout << "init model finish " << "resources/objects/tank/hull.obj" << endl;
	Model track("resources/objects/tank/track.obj");
	cout << "init model finish " << "resources/objects/tank/track.obj" << endl;
	Model turret("resources/objects/tank/turret.obj");
	cout << "init model finish" << "resources/objects/tank/turret.obj" << endl;
	Model wheels("resources/objects/tank/wheels.obj");
	cout << "init model finish " << "resources/objects/tank/wheels.obj" << endl;
	Model floor("resources/objects/tank/floor.obj");
	cout << "init model finish " << "resources/objects/tank/floor.obj" << endl;

	// 定义光源的位置和颜色
	glm::vec3 lightPositions[] = {
		glm::vec3(-10.0f,  10.0f, -10.0f)
	};
	glm::vec3 lightColors[] = {
		glm::vec3(1000.0f, 1000.0f, 1000.0f)
	};

	// PBR: 设置帧缓存
	unsigned int captureFBO;
	unsigned int captureRBO;
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

	// PBR: 加载HDR环境贴图
	stbi_set_flip_vertically_on_load(true);
	int width, height, nrComponents;
	float* data = stbi_loadf("resources/textures/hdr/dancing_hall_4k.hdr", &width, &height, &nrComponents, 0);
	unsigned int hdrTexture;
	if (data)
	{
		// 创建纹理
		glGenTextures(1, &hdrTexture);
		glBindTexture(GL_TEXTURE_2D, hdrTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data); // note how we specify the texture's data value to be float

		// 纹理参数设置
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Failed to load HDR image." << std::endl;
	}

	// PBR: 设置用于渲染的立方贴图并附加到帧缓存
	unsigned int envCubemap;
	glGenTextures(1, &envCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	// 立方贴图的参数设置
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // enable pre-filter mipmap sampling (combatting visible dots artifact)
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// PBR: 为6个立方贴图面方向设置投影和视图矩阵
	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	// 定义6个面的视图矩阵
	glm::mat4 captureViews[] =
	{
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	// PBR: 将HDR等矩形环境贴图转换为立方贴图等效物
	equirectangularToCubemapShader.use();
	equirectangularToCubemapShader.setInt("equirectangularMap", 0);
	equirectangularToCubemapShader.setMat4("projection", captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);

	glViewport(0, 0, 512, 512); // don't forget to configure the viewport to the capture dimensions.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (unsigned int i = 0; i < 6; ++i)
	{
		equirectangularToCubemapShader.setMat4("view", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		renderCube();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// 让OpenGL从第一个mip面生成mipmaps（对抗可见的点伪像）
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	// PBR: 创建辐照率立方图，并将捕捉FBO重新调整为辐照率尺寸。
	unsigned int irradianceMap;
	glGenTextures(1, &irradianceMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		// 为每个立方体贴图面分配内存
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	// 设置纹理参数
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// 绑定帧缓冲对象和渲染缓冲对象
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

	// PBR: 通过卷积解决漫反射积分，创建辐照率（立方图）映射。
	irradianceShader.use();
	irradianceShader.setInt("environmentMap", 0);
	irradianceShader.setMat4("projection", captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

	// 配置视口大小
	glViewport(0, 0, 32, 32);
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (unsigned int i = 0; i < 6; ++i)
	{
		irradianceShader.setMat4("view", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// 渲染立方体
		renderCube();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// PBR: 创建预过滤立方图，并将捕捉FBO重新调整为预过滤尺寸。
	unsigned int prefilterMap;
	glGenTextures(1, &prefilterMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		// 为每个立方体贴图面分配内存
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	// 设置纹理参数
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // be sure to set minification filter to mip_linear 
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// 生成立方图的mipmap
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	// PBR: 对环境光进行准蒙特卡洛模拟，创建预过滤（立方图）映射
	prefilterShader.use();
	prefilterShader.setInt("environmentMap", 0);
	prefilterShader.setMat4("projection", captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	unsigned int maxMipLevels = 5;
	for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
	{
		// 根据mip级别调整帧缓冲对象的大小
		unsigned int mipWidth = static_cast<unsigned int>(128 * std::pow(0.5, mip));
		unsigned int mipHeight = static_cast<unsigned int>(128 * std::pow(0.5, mip));
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
		glViewport(0, 0, mipWidth, mipHeight);

		float roughness = (float)mip / (float)(maxMipLevels - 1);
		prefilterShader.setFloat("roughness", roughness);
		for (unsigned int i = 0; i < 6; ++i)
		{
			prefilterShader.setMat4("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			renderCube();
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// PBR: 从使用的BRDF方程生成2D LUT
	unsigned int brdfLUTTexture;
	glGenTextures(1, &brdfLUTTexture);

	// PBR: 从使用的BRDF方程生成2D LUT。
	glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
	// 设置纹理参数
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// 配置捕捉帧缓冲对象并使用BRDF着色器渲染屏幕空间四边形
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

	glViewport(0, 0, 512, 512);
	brdfShader.use();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderQuad();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	// 在渲染前初始化着色器的uniforms变量
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	pbrShader.use();
	pbrShader.setMat4("projection", projection);
	backgroundShader.use();
	backgroundShader.setMat4("projection", projection);

	// 在渲染前，将视口配置为原始framebuffer的屏幕尺寸
	int scrWidth, scrHeight;
	glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
	glViewport(0, 0, scrWidth, scrHeight);

	glm::vec3 pokeball_translate = glm::vec3(20, 0, -10);
	float pokeball_scale = 1;
	glm::vec3 tank_translate = glm::vec3(0, 0, -10);
	float tank_scale = 10;

	// 渲染循环
	while (!glfwWindowShouldClose(window))
	{
		// 每帧的时间信息
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		
		// 处理输入
		processInput(window);

		// 开始绘制ImGui界面
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// 配置ImGui窗口位置和大小
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(460, 450));
		ImGui::Begin("Options");
		// 显示FPS等信息
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)\n\n", deltaTime * 1000, 1.0f / deltaTime);
		// 显示控制说明
		ImGui::Text("Exit : Esc \n");
		ImGui::Text("Camera Control : W A S D \n");
		ImGui::Text("Cursor Normal : N\n");
		ImGui::Text("Cursor Disabled : M\n\n");
		// 相机设置
		ImGui::Text("Camera Settings:\n");
		// 添加滑块控制相机移动速度
		ImGui::SliderFloat("Camera Speed", &camera.MovementSpeed, 1.0f, 50.0f);
		// 添加滑块控制鼠标灵敏度
		ImGui::SliderFloat("Mouse Sensitivity", &camera.MouseSensitivity, 0.1f, 2.0f);
		// 灯光设置
		ImGui::Text("\nLight Settings:\n");
		for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
		{
			// 修改灯光颜色和位置
			ImGui::ColorEdit3(("Light Color " + std::to_string(i)).c_str(), (float*)&lightColors[i]);
			ImGui::InputFloat3("Light Position", (float*)&lightPositions[i]);
		}
		// pokeball 模型设置
		ImGui::Text("\nPokeBall Settings:\n");
		ImGui::InputFloat3("PokeBall Translate", (float*)&pokeball_translate);
		ImGui::SliderFloat("PokeBall Scale", (float*)&pokeball_scale, 0.1f, 20.0f);
		// tank 模型设置
		ImGui::Text("\nTank Settings:\n");
		ImGui::InputFloat3("Tank Translate", (float*)&tank_translate);
		ImGui::SliderFloat("Tank Scale", (float*)&tank_scale, 0.1f, 20.0f);
		ImGui::End();

		// 渲染
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 使用 pbrShader 进行场景渲染
		pbrShader.use();
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = camera.GetViewMatrix();
		pbrShader.setMat4("view", view);
		pbrShader.setVec3("camPos", camera.Position);

		// 绑定预计算的 IBL 数据
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);

		// 渲染 pokeball 模型
		model = glm::mat4(1.0f);
		model = glm::translate(model, pokeball_translate);
		model = glm::scale(model, glm::vec3(pokeball_scale, pokeball_scale, pokeball_scale));
		renderPbrModel(modelAlbedoMap0, modelNormalMap0, modelMetallicMap0, modelRoughnessMap0, modelAoMapModel0, pbrShader, pokeball, model);

		// 渲染 tank 模型
		model = glm::mat4(1.0f);
		model = glm::translate(model, tank_translate);
		model = glm::scale(model, glm::vec3(tank_scale, tank_scale, tank_scale));
		renderPbrModel(modelAlbedoMap1, modelNormalMap1, modelMetallicMap1, modelRoughnessMap1, modelAoMapModel1, pbrShader, hull, model);
		renderPbrModel(modelAlbedoMap2, modelNormalMap2, modelMetallicMap2, modelRoughnessMap2, modelAoMapModel2, pbrShader, track, model);
		renderPbrModel(modelAlbedoMap3, modelNormalMap3, modelMetallicMap3, modelRoughnessMap3, modelAoMapModel3, pbrShader, turret, model);
		renderPbrModel(modelAlbedoMap4, modelNormalMap4, modelMetallicMap4, modelRoughnessMap4, modelAoMapModel4, pbrShader, wheels, model);
		renderPbrModel(modelAlbedoMap5, modelNormalMap5, modelMetallicMap5, modelRoughnessMap5, modelAoMapModel5, pbrShader, floor, model);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, goldAlbedoMap);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, goldNormalMap);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, goldMetallicMap);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, goldRoughnessMap);
		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, goldAOMap);
		// 渲染光源
		for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
		{
			glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0);
			newPos = lightPositions[i];
			pbrShader.setVec3("lightPositions[" + std::to_string(i) + "]", newPos);
			pbrShader.setVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);

			model = glm::mat4(1.0f);
			model = glm::translate(model, newPos);
			model = glm::scale(model, glm::vec3(0.5f));
			pbrShader.setMat4("model", model);
			pbrShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
			// 渲染光源形状为球体
			renderSphere();
		}

		// 渲染天空盒，作为背景
		backgroundShader.use();
		backgroundShader.setMat4("view", view);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
		renderCube();

		// 渲染ImGui的绘制数据
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// glfw: 交换缓冲并查询IO事件 (如键盘按下/释放，鼠标移动等)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// glfw：终止并清除之前分配的所有glfw资源
	glfwTerminate();
	return 0;
}

// 处理所有输入：查询GLFW是否在这个帧中按下/释放了相关的按键，并做出相应的反应
void processInput(GLFWwindow* window)
{
	// 当按下“Escape”键时，关闭窗口
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// 当按下“Escape”键时，关闭窗口
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);

	// 当N、M键被按下时，改变光标模式
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

// 当窗口大小发生改变（由操作系统或用户调整大小）时，执行此回调函数
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// 确保视口与新窗口的尺寸匹配；注意在视网膜显示器上，宽度和高度会比指定的大很多
	glViewport(0, 0, width, height);
}

// 当鼠标移动时，调用此回调函数
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	// 当鼠标移动时，调用此回调函数
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// 当鼠标滚轮滚动时，调用此回调函数
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// 渲染（并在第一次调用时构建）一个已加载的PBR模型
void renderPbrModel(unsigned int albedoMap, unsigned int normalMap, unsigned int metallicMap, unsigned int roughnessMap, unsigned int aoMap, Shader& pbrShader, Model inputModel, glm::mat4 model)
{
	// 设置PBR纹理
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, albedoMap);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, normalMap);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, metallicMap);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, roughnessMap);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, aoMap);

	pbrShader.setMat4("model", model);
	inputModel.Draw(pbrShader);
}

// 首次调用时构建并渲染一个球体
unsigned int sphereVAO = 0;
GLsizei indexCount;
void renderSphere()
{
	// 如果sphereVAO是0，即第一次调用此函数，生成球体的顶点数据
	if (sphereVAO == 0)
	{
		glGenVertexArrays(1, &sphereVAO);

		unsigned int vbo, ebo;
		glGenBuffers(1, &vbo);
		glGenBuffers(1, &ebo);

		std::vector<glm::vec3> positions;
		std::vector<glm::vec2> uv;
		std::vector<glm::vec3> normals;
		std::vector<unsigned int> indices;

		const unsigned int X_SEGMENTS = 64;
		const unsigned int Y_SEGMENTS = 64;
		const float PI = 3.14159265359f;
		// 生成球体的顶点数据
		for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
		{
			for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
			{
				float xSegment = (float)x / (float)X_SEGMENTS;
				float ySegment = (float)y / (float)Y_SEGMENTS;
				float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
				float yPos = std::cos(ySegment * PI);
				float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

				positions.push_back(glm::vec3(xPos, yPos, zPos));
				uv.push_back(glm::vec2(xSegment, ySegment));
				normals.push_back(glm::vec3(xPos, yPos, zPos));
			}
		}
		// 根据奇偶行来确定顶点的连接顺序
		bool oddRow = false;
		for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
		{
			if (!oddRow) // even rows: y == 0, y == 2; and so on
			{
				for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
				{
					indices.push_back(y * (X_SEGMENTS + 1) + x);
					indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
				}
			}
			else
			{
				for (int x = X_SEGMENTS; x >= 0; --x)
				{
					indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
					indices.push_back(y * (X_SEGMENTS + 1) + x);
				}
			}
			oddRow = !oddRow;
		}
		indexCount = static_cast<GLsizei>(indices.size());

		std::vector<float> data;
		for (unsigned int i = 0; i < positions.size(); ++i)
		{
			data.push_back(positions[i].x);
			data.push_back(positions[i].y);
			data.push_back(positions[i].z);
			if (normals.size() > 0)
			{
				data.push_back(normals[i].x);
				data.push_back(normals[i].y);
				data.push_back(normals[i].z);
			}
			if (uv.size() > 0)
			{
				data.push_back(uv[i].x);
				data.push_back(uv[i].y);
			}
		}
		// 绑定和设置OpenGL缓冲区
		glBindVertexArray(sphereVAO);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
		unsigned int stride = (3 + 2 + 3) * sizeof(float);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
	}
	// 绘制球体
	glBindVertexArray(sphereVAO);
	glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
}

// renderCube() 函数用于渲染一个1x1的3D立方体在NDC中
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
	// 如果需要的话，进行初始化
	if (cubeVAO == 0)
	{
		float vertices[] = {
			// back face
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
			// front face
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			// left face
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			// right face
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
			 // bottom face
			 -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			  1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
			  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			 -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			 -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			 // top face
			 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			  1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			  1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
			  1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			 -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// 填充缓冲
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// 链接顶点属性
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// 渲染立方体
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

// renderQuad() 函数用于渲染一个1x1的XY平面在NDC中
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// 位置        // 纹理坐标
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// 设置平面的VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

// 用于从文件加载2D纹理的函数
unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}
