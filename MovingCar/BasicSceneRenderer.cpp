#include "BasicSceneRenderer.h"
#include "Image.h"
#include "Prefabs.h"
#include <GL/glut.h>
#include <iostream>

GLfloat d;
GLfloat p1x;
GLfloat p1y;
GLfloat p1z;
const int p1radius = 1;
const int p2radius = 0;

GLfloat p2x;
GLfloat p2y;
GLfloat p2z;

BasicSceneRenderer::BasicSceneRenderer()
    : mLightingModel(PER_VERTEX_DIR_LIGHT)
    , mCamera(NULL)
    , mProjMatrix(1.0f)
    , mActiveEntityIndex(0)
    , mDbgProgram(NULL)
    , mAxes(NULL)
    , mVisualizePointLights(true)
{}

void calculateDistance() {
	//set the distance to the current distance between car and obstacle
	//TODO: update for collision with more than one entity
	d = sqrt(((p1x - p2x) * (p1x - p2x)) + ((p1y - p2y) * (p1y - p2y)) + ((p1z - p2z) * (p1z - p2z)));
}

void BasicSceneRenderer::initialize()
{
    // print usage instructions
    std::cout << "Usage:" << std::endl;
    std::cout << "  Camera control:           WASD + Mouse" << std::endl;
    std::cout << "  Rotate active entity:     Arrow keys" << std::endl;
    std::cout << "  Reset entity orientation: R" << std::endl;
    std::cout << "  Translate active entity:  IJKL (world space)" << std::endl;
    std::cout << "  Translate active entity:  TFGH (local space)" << std::endl;
    std::cout << "  Cycle active entity:      X/Z" << std::endl;
    std::cout << "  Toggle point light vis.:  Tab" << std::endl;

    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // enable blending (needed for textures with alpha channel)
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    mPrograms.resize(NUM_LIGHTING_MODELS);

    mPrograms[PER_VERTEX_DIR_LIGHT] = new ShaderProgram("shaders/PerVertexDirLight-vs.glsl",
                                                                "shaders/PerVertexDirLight-fs.glsl");
    
    mPrograms[BLINN_PHONG_PER_FRAGMENT_DIR_LIGHT] = new ShaderProgram("shaders/BlinnPhongPerFragment-vs.glsl",
                                                                      "shaders/BlinnPhongPerFragmentDirLight-fs.glsl");

    mPrograms[BLINN_PHONG_PER_FRAGMENT_POINT_LIGHT] = new ShaderProgram("shaders/BlinnPhongPerFragment-vs.glsl",
                                                                        "shaders/BlinnPhongPerFragmentPointLight-fs.glsl");

    mPrograms[BLINN_PHONG_PER_FRAGMENT_MULTI_LIGHT] = new ShaderProgram("shaders/BlinnPhongPerFragment-vs.glsl",
                                                                        "shaders/BlinnPhongPerFragmentMultiLight-fs.glsl");

    //
    // Create meshes
    //
	//TODO: Mesh Creation starts here
    mMeshes.push_back(CreateTexturedCube(1));
    mMeshes.push_back(CreateChunkyTexturedCylinder(1.5f, 1, 8));
    mMeshes.push_back(CreateSmoothTexturedCylinder(0.5f, 1, 15));

    float roomWidth = 12;
    float roomHeight = 24;
    float roomDepth = 64;
    float roomTilesPerUnit = 0.25f;

    // front and back walls
    Mesh* fbMesh = CreateTexturedQuad(roomWidth, roomHeight, roomWidth * roomTilesPerUnit, roomHeight * roomTilesPerUnit);
    mMeshes.push_back(fbMesh);
    // left and right walls
    Mesh* lrMesh = CreateTexturedQuad(roomDepth, roomHeight, roomDepth * roomTilesPerUnit, roomHeight * roomTilesPerUnit);
    mMeshes.push_back(lrMesh);
    // ceiling and floor
    Mesh* cfMesh = CreateTexturedQuad(roomWidth, roomDepth, roomWidth * roomTilesPerUnit, roomDepth * roomTilesPerUnit);
    mMeshes.push_back(cfMesh);

    //
    // Load textures
    //

    std::vector<std::string> texNames;
    texNames.push_back("textures/CarvedSandstone.tga");
    texNames.push_back("textures/rocky.tga");
    texNames.push_back("textures/bricks_overpainted_blue_9291383.tga");
    texNames.push_back("textures/water_drops_on_metal_3020602.tga");
    texNames.push_back("textures/skin.tga");
    texNames.push_back("textures/white.tga");
    texNames.push_back("textures/yo.tga");
    texNames.push_back("textures/black.tga");

    for (unsigned i = 0; i < texNames.size(); i++)
        mTextures.push_back(new Texture(texNames[i], GL_REPEAT, GL_LINEAR));

    //
    // Create materials
    //

    // add a material for each loaded texture (with default tint)
    for (unsigned i = 0; i < texNames.size(); i++)
        mMaterials.push_back(new Material(mTextures[i]));

    //
    // set extra material properties
    //

    // water drops (sharp and strong specular highlight)
    mMaterials[3]->specular = glm::vec3(1.0f, 1.0f, 1.0f);
    mMaterials[3]->shininess = 128;

    // skin (washed out and faint specular highlight)
    mMaterials[4]->specular = glm::vec3(0.3f, 0.3f, 0.3f);
    mMaterials[4]->shininess = 8;

    // white
    mMaterials[5]->specular = glm::vec3(0.75f, 0.75f, 0.75f);
    mMaterials[5]->shininess = 64;

    // yo
    mMaterials[6]->specular = glm::vec3(1.0f, 0.0f, 1.0f);  // magenta highlights
    mMaterials[6]->shininess = 16;

    // black
    mMaterials[7]->specular = glm::vec3(1.0f, 0.5f, 0.0f);  // orange hightlights
    mMaterials[7]->shininess = 16;

    //
    // Create entities
    //

    unsigned numRows = mMaterials.size();
    float spacing = 3;
    float z = 0.5f * spacing * numRows;
	//TODO: Car and Obstacles created here
	//car
	mEntities.push_back(new Entity(mMeshes[0], mMaterials[3], Transform(0.0f, -11.5f, 30)));
	//test obstacle
	mEntities.push_back(new Entity(mMeshes[1], mMaterials[2], Transform(2.0f, -11.5f, -5)));
	z -= spacing;
	/*
	for (unsigned i = 2; i < mMaterials.size(); i++) {
        // cube
        mEntities.push_back(new Entity(mMeshes[0], mMaterials[i], Transform(-4.0f, 0.0f, z)));
        // chunky cylinder
        mEntities.push_back(new Entity(mMeshes[1], mMaterials[i], Transform( 0.0f, 0.0f, z)));
        // smooth cylinder
        mEntities.push_back(new Entity(mMeshes[2], mMaterials[i], Transform( 4.0f, 0.0f, z)));

        // next row
        z -= spacing;
    }
	*/
    //
    // Create room
    //

    // back wall
    /*mEntities.push_back(new Entity(fbMesh, mMaterials[1], Transform(0, 0, -0.5f * roomDepth)));
    // front wall
    mEntities.push_back(new Entity(fbMesh, mMaterials[1], Transform(0, 0, 0.5f * roomDepth, glm::angleAxis(glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)))));
    // left wall
    mEntities.push_back(new Entity(lrMesh, mMaterials[1], Transform(-0.5f * roomWidth, 0, 0, glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)))));
    // right wall
    mEntities.push_back(new Entity(lrMesh, mMaterials[1], Transform(0.5f * roomWidth, 0, 0, glm::angleAxis(glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)))));*/
    // floor
    mEntities.push_back(new Entity(cfMesh, mMaterials[1], Transform(0, -0.5f * roomHeight, 0, glm::angleAxis(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)))));
    // ceiling
    //mEntities.push_back(new Entity(cfMesh, mMaterials[0], Transform(0, 0.5f * roomHeight, 0, glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)))));

    //
    // create the camera
    //

    mCamera = new Camera(this);
    mCamera->setPosition(0, -8, 40);
    mCamera->lookAt(0, -8, 0);
    mCamera->setSpeed(6);

    // create shader program for debug geometry
    mDbgProgram = new ShaderProgram("shaders/vpc-vs.glsl",
                                    "shaders/vcolor-fs.glsl");

    // create geometry for axes
    mAxes = CreateAxes(2);

    CHECK_GL_ERRORS("initialization");
}

void BasicSceneRenderer::shutdown()
{
    for (unsigned i = 0; i < mPrograms.size(); i++)
        delete mPrograms[i];
    mPrograms.clear();

    delete mDbgProgram;
    mDbgProgram = NULL;

    delete mCamera;
    mCamera = NULL;

    for (unsigned i = 0; i < mEntities.size(); i++)
        delete mEntities[i];
    mEntities.clear();

    for (unsigned i = 0; i < mMeshes.size(); i++)
        delete mMeshes[i];
    mMeshes.clear();

    for (unsigned i = 0; i < mMaterials.size(); i++)
        delete mMaterials[i];
    mMaterials.clear();
    
    for (unsigned i = 0; i < mTextures.size(); i++)
        delete mTextures[i];
    mTextures.clear();

    delete mDbgProgram;
    mDbgProgram = NULL;
    
    delete mAxes;
    mAxes = NULL;
}

void BasicSceneRenderer::resize(int width, int height)
{
    glViewport(0, 0, width, height);

    // compute new projection matrix
    mProjMatrix = glm::perspective(glm::radians(50.f), width / (float)height, 0.1f, 1000.0f);
}

void BasicSceneRenderer::draw()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // activate current program
    ShaderProgram* prog = mPrograms[mLightingModel];
    prog->activate();

    // send projection matrix
    prog->sendUniform("u_ProjectionMatrix", mProjMatrix);

    // send the texture sampler id to shader
    prog->sendUniformInt("u_TexSampler", 0);

    // get the view matrix from the camera
    glm::mat4 viewMatrix = mCamera->getViewMatrix();

    //
    // light setup depends on lighting model
    //

    if (mLightingModel == PER_VERTEX_DIR_LIGHT) {

        //----------------------------------------------------------------------------------//
        //                                                                                  //
        // Basic directional light (no ambient, specular, or emissive contributions)        //
        //                                                                                  //
        //----------------------------------------------------------------------------------//

        // direction to light
        glm::vec4 lightDir = glm::normalize(glm::vec4(1, 3, 2, 0));

        // send light direction in eye space
        prog->sendUniform("u_LightDir", glm::vec3(viewMatrix * lightDir));

        // send light color/intensity
        prog->sendUniform("u_LightColor", glm::vec3(1.0f, 1.0f, 1.0f));

    } /*else if (mLightingModel == BLINN_PHONG_PER_FRAGMENT_DIR_LIGHT) {

        //----------------------------------------------------------------------------------//
        //                                                                                  //
        // Directional light with ambient, specular, and emissive contributions             //
        //                                                                                  //
        //----------------------------------------------------------------------------------//

        prog->sendUniform("u_AmbientLightColor", glm::vec3(0.2f, 0.2f, 0.2f));

        // direction to light
        glm::vec4 lightDir = glm::normalize(glm::vec4(1, 3, 2, 0));

        // send light direction in eye space
        prog->sendUniform("u_LightDir", glm::vec3(viewMatrix * lightDir));

        // send light color/intensity
        prog->sendUniform("u_LightColor", glm::vec3(0.8f, 0.8f, 0.8f));

    } else if (mLightingModel == BLINN_PHONG_PER_FRAGMENT_POINT_LIGHT) {

        //----------------------------------------------------------------------------------//
        //                                                                                  //
        // Point light with ambient, specular, and emissive contributions, and attenuation  //
        //                                                                                  //
        //----------------------------------------------------------------------------------//

        prog->sendUniform("u_AmbientLightColor", glm::vec3(0.1f, 0.1f, 0.1f));

        // point light position
        glm::vec3 lightPos = glm::vec3(0, 7, 0);
        glm::vec3 lightColor = glm::vec3(1.0f, 0.9f, 0.8f);

        // send light position in eye space
        prog->sendUniform("u_LightPos", glm::vec3(viewMatrix * glm::vec4(lightPos, 1)));

        // send light color/intensity
        prog->sendUniform("u_LightColor", lightColor);

        prog->sendUniform("u_AttQuat", 0.005f);
        prog->sendUniform("u_AttLin", 0.05f);
        prog->sendUniform("u_AttConst", 1.0f);

        // render the light as an emissive cube, if desired
        if (mVisualizePointLights) {
            const Mesh* lightMesh = mMeshes[0];
            lightMesh->activate();
            glBindTexture(GL_TEXTURE_2D, mTextures[7]->id());  // use black texture
            prog->sendUniform("u_MatEmissiveColor", lightColor);
            prog->sendUniform("u_ModelviewMatrix", glm::translate(viewMatrix, glm::vec3(lightPos)));
            prog->sendUniform("u_NormalMatrix", glm::mat3(1.0f));
            lightMesh->draw();
        }

    }*/ /*else if (mLightingModel == BLINN_PHONG_PER_FRAGMENT_MULTI_LIGHT) {

        //----------------------------------------------------------------------------------//
        //                                                                                  //
        // Multiple directional/point lights                                                //
        //                                                                                  //
        //----------------------------------------------------------------------------------//

        prog->sendUniform("u_AmbientLightColor", glm::vec3(0.1f, 0.1f, 0.1f));

        prog->sendUniformInt("u_NumDirLights", 1);
        prog->sendUniformInt("u_NumPointLights", 3);

        // directional light
        glm::vec4 lightDir = glm::normalize(glm::vec4(1, 3, 2, 0));
        prog->sendUniform("u_DirLights[0].dir", glm::vec3(viewMatrix * lightDir));
        prog->sendUniform("u_DirLights[0].color", glm::vec3(0.3f, 0.3f, 0.3f));

        // point light
        glm::vec3 lightPos1 = glm::vec3(-7, 5, -12);
        glm::vec3 lightColor1 = glm::vec3(1.0f, 0.0f, 0.0f);
        prog->sendUniform("u_PointLights[0].pos", glm::vec3(viewMatrix * glm::vec4(lightPos1, 1)));
        prog->sendUniform("u_PointLights[0].color", lightColor1);
        prog->sendUniform("u_PointLights[0].attQuat", 0.01f);
        prog->sendUniform("u_PointLights[0].attLin", 0.1f);
        prog->sendUniform("u_PointLights[0].attConst", 1.0f);

        // point light
        glm::vec3 lightPos2 = glm::vec3(7, 5, -12);
        glm::vec3 lightColor2 = glm::vec3(0.0f, 0.0f, 1.0f);
        prog->sendUniform("u_PointLights[1].pos", glm::vec3(viewMatrix * glm::vec4(lightPos2, 1)));
        prog->sendUniform("u_PointLights[1].color", lightColor2);
        prog->sendUniform("u_PointLights[1].attQuat", 0.01f);
        prog->sendUniform("u_PointLights[1].attLin", 0.1f);
        prog->sendUniform("u_PointLights[1].attConst", 1.0f);

        // point light
        glm::vec3 lightPos3 = glm::vec3(-7, -5, 15);
        glm::vec3 lightColor3 = glm::vec3(0.0f, 1.0f, 0.0f);
        prog->sendUniform("u_PointLights[2].pos", glm::vec3(viewMatrix * glm::vec4(lightPos3, 1)));
        prog->sendUniform("u_PointLights[2].color", lightColor3);
        prog->sendUniform("u_PointLights[2].attQuat", 0.01f);
        prog->sendUniform("u_PointLights[2].attLin", 0.1f);
        prog->sendUniform("u_PointLights[2].attConst", 1.0f);

        // render the point lights as emissive cubes, if desirable
        if (mVisualizePointLights) {
            glBindTexture(GL_TEXTURE_2D, mTextures[7]->id());  // use black texture
            prog->sendUniform("u_NormalMatrix", glm::mat3(1.0f));
            const Mesh* lightMesh = mMeshes[0];
            lightMesh->activate();
            prog->sendUniform("u_MatEmissiveColor", lightColor1);
            prog->sendUniform("u_ModelviewMatrix", glm::translate(viewMatrix, glm::vec3(lightPos1)));
            lightMesh->draw();
            prog->sendUniform("u_MatEmissiveColor", lightColor2);
            prog->sendUniform("u_ModelviewMatrix", glm::translate(viewMatrix, glm::vec3(lightPos2)));
            lightMesh->draw();
            prog->sendUniform("u_MatEmissiveColor", lightColor3);
            prog->sendUniform("u_ModelviewMatrix", glm::translate(viewMatrix, glm::vec3(lightPos3)));
            lightMesh->draw();
        }
    }*/

    // render all entities
    for (unsigned i = 0; i < mEntities.size(); i++) {

        Entity* ent = mEntities[i];

        // use the entity's material
        const Material* mat = ent->getMaterial();
        glBindTexture(GL_TEXTURE_2D, mat->tex->id());   // bind texture
        prog->sendUniform("u_Tint", mat->tint);     // send tint color

        // send the Blinn-Phong parameters, if required
        if (mLightingModel > PER_VERTEX_DIR_LIGHT) {
            prog->sendUniform("u_MatEmissiveColor", mat->emissive);
            prog->sendUniform("u_MatSpecularColor", mat->specular);
            prog->sendUniform("u_MatShininess", mat->shininess);
        }

        // compute modelview matrix
        glm::mat4 modelview = viewMatrix * ent->getWorldMatrix();

        // send the entity's modelview and normal matrix
        prog->sendUniform("u_ModelviewMatrix", modelview);
        prog->sendUniform("u_NormalMatrix", glm::transpose(glm::inverse(glm::mat3(modelview))));

        // use the entity's mesh
        const Mesh* mesh = ent->getMesh();
        mesh->activate();
        mesh->draw();
    }

    //
    // draw local axes for current entity
    //

    mDbgProgram->activate();
    mDbgProgram->sendUniform("u_ProjectionMatrix", mProjMatrix);

    Entity* activeEntity = mEntities[mActiveEntityIndex];
    mDbgProgram->sendUniform("u_ModelviewMatrix", viewMatrix * activeEntity->getWorldMatrix());
    mAxes->activate();
    //comment out the line below to get rid of axes
	mAxes->draw();

    CHECK_GL_ERRORS("drawing");
}

bool BasicSceneRenderer::update(float dt)
{
	//TODO: Update starts here
	const Keyboard* kb = getKeyboard();

	if (kb->keyPressed(KC_ESCAPE))
		return false;

	// move forward through our list of entities
	/*if (kb->keyPressed(KC_Q)) {
		// compute next entity index
		++mActiveEntityIndex;
		if (mActiveEntityIndex >= (int)mEntities.size())
			mActiveEntityIndex = 0;
	}

	// move backward through our list of entities
	if (kb->keyPressed(KC_Z)) {
		// compute previous entity index
		--mActiveEntityIndex;
		if (mActiveEntityIndex < 0)
			mActiveEntityIndex = (int)mEntities.size() - 1;
	}*/

	// get the entity to manipulate
	Entity* activeEntity = mEntities[mActiveEntityIndex];
	//player is the first thing we render so it's mEntities 0
	Entity* playerVehicle = mEntities[0];
	//test obstacle
	Entity* obstacle = mEntities[1];
	//position of the car, can use carPos.x, etc. for coordinates
	glm::vec3 carPos = playerVehicle->getPosition();
	//position of test obstacle
	glm::vec3 obsPos = obstacle->getPosition();
	//camera position vector
	glm::vec3 cameraPosition = mCamera->getPosition();

	// rotate the entity
	float rotSpeed = 90;
	float rotAmount = rotSpeed * dt;

	p1x = carPos.x;
	p1y = carPos.y;
	p1z = carPos.z;
	p2x = obsPos.x;
	p2y = obsPos.y;
	p2z = obsPos.z;
	
	calculateDistance();
	
	//movement controls for the car, forward, backward, left, right
	//if key's pressed
	if (kb->isKeyDown(KC_LEFT)) {
		/*
		recalculate distance
		add 0.1 to distance
		then if distance is less than 1
		*/
		//activeEntity->rotate(rotAmount, 0, 1, 0);
		if (d < 2) {
			d += 0.1;
			std::cout << d << std::endl;
			playerVehicle->translateLocal(0.15, 0, 0);
		}
		else{ 
			playerVehicle->translateLocal(-0.1, 0, 0);
		}
	}
	if (kb->isKeyDown(KC_RIGHT)) {
		//activeEntity->rotate(-rotAmount, 0, 1, 0);
		if (d<2) { 
			d += 0.1;
			std::cout << d << std::endl;
			playerVehicle->translateLocal(-0.15, 0, 0);
		}
		else { playerVehicle->translateLocal(0.1, 0, 0); }
		
	}
	if (kb->isKeyDown(KC_Z) || kb->isKeyDown(KC_UP)) {
		if (d < 2) { 
			d += 0.1;
			std::cout << d << std::endl;
			//playerVehicle->translateLocal(0, 0, 0.1);
			mCamera->setPosition(cameraPosition.x, cameraPosition.y, cameraPosition.z);
		}
		else { 
			playerVehicle->translateLocal(0, 0, -0.2); 
			mCamera->setPosition(cameraPosition.x, cameraPosition.y, cameraPosition.z - 0.2);
		}
		
	}
	if (kb->isKeyDown(KC_X)|| kb->isKeyDown(KC_DOWN)) {
		if (d < 2) { 
			d += 0.1;
			std::cout << d << std::endl; 
			//playerVehicle->translateLocal(0, 0, -0.1);
			mCamera->setPosition(cameraPosition.x, cameraPosition.y, cameraPosition.z);
		}
		else { 
			playerVehicle->translateLocal(0, 0, 0.2); 
			mCamera->setPosition(cameraPosition.x, cameraPosition.y, cameraPosition.z + 0.2);
		}
		
	}

    /*if (kb->isKeyDown(KC_UP))
        activeEntity->rotate(rotAmount, 1, 0, 0);
    if (kb->isKeyDown(KC_DOWN))
        activeEntity->rotate(-rotAmount, 1, 0, 0);*/

    // reset entity orientation
    /*if (kb->keyPressed(KC_R))
        activeEntity->setOrientation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f));

    float speed = 3;
    float disp = speed * dt;

    // move entity along world axes
    if (kb->isKeyDown(KC_I))
        activeEntity->translate(0, 0, disp);
    if (kb->isKeyDown(KC_K))
        activeEntity->translate(0, 0, -disp);
    if (kb->isKeyDown(KC_L))
        activeEntity->translate(disp, 0, 0);
    if (kb->isKeyDown(KC_J))
        activeEntity->translate(-disp, 0, 0);

    // move entity along entity's local axes
    if (kb->isKeyDown(KC_T))
        activeEntity->translateLocal(0, 0, disp);
    if (kb->isKeyDown(KC_G))
        activeEntity->translateLocal(0, 0, -disp);
    if (kb->isKeyDown(KC_F))
        activeEntity->translateLocal(disp, 0, 0);
    if (kb->isKeyDown(KC_H))
        activeEntity->translateLocal(-disp, 0, 0);

    // change lighting models
    if (kb->keyPressed(KC_1))
        mLightingModel = PER_VERTEX_DIR_LIGHT;
    if (kb->keyPressed(KC_2))
        mLightingModel = BLINN_PHONG_PER_FRAGMENT_DIR_LIGHT;
    if (kb->keyPressed(KC_3))
        mLightingModel = BLINN_PHONG_PER_FRAGMENT_POINT_LIGHT;
    if (kb->keyPressed(KC_4))
        mLightingModel = BLINN_PHONG_PER_FRAGMENT_MULTI_LIGHT;

    // toggle visualization of point lights
    if (kb->keyPressed(KC_TAB))
        mVisualizePointLights = !mVisualizePointLights;*/

    // update the camera
    mCamera->update(dt);

    return true;
}