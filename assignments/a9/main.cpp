#include "Common.h"
#include "OpenGLCommon.h"
#include "OpenGLMarkerObjects.h"
#include "OpenGLBgEffect.h"
#include "OpenGLMesh.h"
#include "OpenGLViewer.h"
#include "OpenGLWindow.h"
#include "TinyObjLoader.h"
#include "OpenGLSkybox.h"
#include <algorithm>
#include <iostream>
#include <random>
#include <unordered_set>
#include <vector>
#include <string>
#include <cmath>

#include <cstdlib> // For rand() and srand()
#include <ctime>   // For time()

#ifndef __Main_cpp__
#define __Main_cpp__

#ifdef __APPLE__
#define CLOCKS_PER_SEC 100000
#endif

#define PI 3.1415926535
#define NUM_FIRE_PARTICLES 200

class Fire 
{
    OpenGLTriangleMesh *obj;
    clock_t time;
    std::vector<double> initVel;
    Matrix4f initTransform;
    public:
        Fire(OpenGLTriangleMesh *mesh, clock_t t, std::vector<double> vel, Matrix4f transform)
        {
        
            obj = mesh;
            time = t;
            initVel = vel;
            initTransform = transform;
        }

        OpenGLTriangleMesh* Fire::getObj()
        {
            return obj;
        }

        clock_t Fire::getTime()
        {
            return time;
        }

        std::vector<double> Fire::getVel()
        {
            return initVel;
        }

        Matrix4f Fire::getTransform()
        {
            return initTransform;
        }

        void Fire::setObj(OpenGLTriangleMesh *mesh)
        {
            obj = mesh;
        }

        void Fire::setTime(clock_t t)
        {
            time = t;
        }

        void Fire::setVel(std::vector<double> vel)
        {
            initVel = vel;
        }

        void Fire::setTransform(Matrix4f transform)
        {
            initTransform = transform;
        }
};

class MyDriver : public OpenGLViewer
{
    std::vector<OpenGLTriangleMesh *> mesh_object_array;
    OpenGLBgEffect *bgEffect = nullptr;
    OpenGLSkybox *skybox = nullptr;
    clock_t startTime;
    float groundLevel = -10.;
    int snowNum = 200;
    std::vector<Fire> fireParticles;
public:
    virtual void Initialize()
    {
        draw_axes = false;
        startTime = clock();
        OpenGLViewer::Initialize();
    }

    virtual void Initialize_Data()
    {
        //// Load all the shaders you need for the scene 
        //// In the function call of Add_Shader_From_File(), we specify three names: 
        //// (1) vertex shader file name
        //// (2) fragment shader file name
        //// (3) shader name used in the shader library
        //// When we bind a shader to an object, we implement it as follows:
        //// object->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("shader_name"));
        //// Here "shader_name" needs to be one of the shader names you created previously with Add_Shader_From_File()

        OpenGLShaderLibrary::Instance()->Add_Shader_From_File("shaders/basic.vert", "shaders/basic.frag", "basic");
        OpenGLShaderLibrary::Instance()->Add_Shader_From_File("shaders/basic.vert", "shaders/environment.frag", "environment");
        OpenGLShaderLibrary::Instance()->Add_Shader_From_File("shaders/stars.vert", "shaders/stars.frag", "stars");
        OpenGLShaderLibrary::Instance()->Add_Shader_From_File("shaders/basic.vert", "shaders/alphablend.frag", "blend");
        OpenGLShaderLibrary::Instance()->Add_Shader_From_File("shaders/billboard.vert", "shaders/alphablend.frag", "billboard");
        OpenGLShaderLibrary::Instance()->Add_Shader_From_File("shaders/terrain.vert", "shaders/terrain.frag", "terrain");
        OpenGLShaderLibrary::Instance()->Add_Shader_From_File("shaders/skybox.vert", "shaders/skybox.frag", "skybox");

        //// Load all the textures you need for the scene
        //// In the function call of Add_Shader_From_File(), we specify two names:
        //// (1) the texture's file name
        //// (2) the texture used in the texture library
        //// When we bind a texture to an object, we implement it as follows:
        //// object->Add_Texture("tex_sampler", OpenGLTextureLibrary::Get_Texture("tex_name"));
        //// Here "tex_sampler" is the name of the texture sampler2D you used in your shader, and
        //// "tex_name" needs to be one of the texture names you created previously with Add_Texture_From_File()

        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/earth_color.png", "sphere_color");
        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/earth_normal.png", "sphere_normal");
        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/bunny_color.jpg", "bunny_color");
        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/bunny_normal.png", "bunny_normal");
        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/window.png", "window_color");
        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/buzz_color.png", "buzz_color");
        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/star_unoriginal3.png", "snow_color");

        // OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/snow_color.png", "snow_color");


        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/star.png", "star_color");
        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/Campfire_MAT_BaseColor_00.jpg", "campfire_color");
        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/Campfire_MAT_Normal_DX.jpg", "campfire_normal");
        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/fire.png", "fire_color");
        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/Final_Tent_Tent_BaseColor.png", "tent_color");
        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/Final_Tent_Tent_Normal.png", "tent_normal");

        //// Add all the lights you need for the scene (no more than 4 lights)
        //// The four parameters are position, ambient, diffuse, and specular.
        //// The lights you declared here will be synchronized to all shaders in uniform lights.
        //// You may access these lights using lt[0].pos, lt[1].amb, lt[1].dif, etc.
        //// You can also create your own lights by directly declaring them in a shader without using Add_Light().
        //// Here we declared three default lights for you. Feel free to add/delete/change them at your will.

        opengl_window->Add_Light(Vector3f(3, 1, 3), Vector3f(0.1f, 0.1f, 0.1f), Vector3f(1, 1, 1), Vector3f(0.5f, 0.5f, 0.5f)); 
        opengl_window->Add_Light(Vector3f(0, 0, -5), Vector3f(0.1f, 0.1f, 0.1f), Vector3f(0.9f, 0.9f, 0.9f), Vector3f(0.5f, 0.5f, 0.5f));
        opengl_window->Add_Light(Vector3f(-5, 1, 3), Vector3f(0.1f, 0.1f, 0.1f), Vector3f(0.9f, 0.9f, 0.9f), Vector3f(0.5f, 0.5f, 0.5f));
        // opengl_window->Add_Light(Vector3f(1, 0, -3), Vector3f(0.1f, 0.1f, 0.1f), Vector3f(0.9f, 0.9f, 0.9f), Vector3f(0.5f, 0.5f, 0.5f));

        //// Add the background / environment
        //// Here we provide you with four default options to create the background of your scene:
        //// (1) Gradient color (like A1 and A2; if you want a simple background, use this one)
        //// (2) Programmable Canvas (like A7 and A8; if you consider implementing noise or particles for the background, use this one)
        //// (3) Sky box (cubemap; if you want to load six background images for a skybox, use this one)
        //// (4) Sky sphere (if you want to implement a sky sphere, enlarge the size of the sphere to make it colver the entire scene and update its shaders for texture colors)
        //// By default, Option (2) (Buzz stars) is turned on, and all the other three are commented out.
        
        //// Background Option (1): Gradient color
        
        {
            auto bg = Add_Interactive_Object<OpenGLBackground>();
            bg->Set_Color(OpenGLColor(0.1f, 0.1f, 0.1f, 1.f), OpenGLColor(0.3f, 0.1f, .1f, 1.f));
            bg->Initialize();
        }
        

        //// Background Option (2): Programmable Canvas
        //// By default, we load a GT buzz + a number of stars
        // {
        //     bgEffect = Add_Interactive_Object<OpenGLBgEffect>();
        //     bgEffect->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("stars"));
        //     bgEffect->Add_Texture("tex_buzz", OpenGLTextureLibrary::Get_Texture("buzz_color")); // bgEffect can also Add_Texture
        //     bgEffect->Initialize();
        // }
        
        //// Background Option (3): Sky box
        //// Here we provide a default implementation of a sky box; customize it for your own sky box
        
        {
            // from https://www.humus.name/index.php?page=Textures
            const std::vector<std::string> cubemap_files{
                "cubemap/posx.jpg",     //// + X
                "cubemap/negx.jpg",     //// - X
                "cubemap/posy.jpg",     //// + Y
                "cubemap/negy.jpg",     //// - Y
                "cubemap/posz.jpg",     //// + Z
                "cubemap/negz.jpg",     //// - Z 
            };
            OpenGLTextureLibrary::Instance()->Add_CubeMap_From_Files(cubemap_files, "cube_map");

            skybox = Add_Interactive_Object<OpenGLSkybox>();
            skybox->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("skybox"));
            skybox->Initialize();
        }
        

        //// Background Option (4): Sky sphere
        //// Here we provide a default implementation of a textured sphere; customize it for your own sky sphere
        // {
        //     //// create object by reading an obj mesh
        //     auto sphere = Add_Obj_Mesh_Object("obj/sphere.obj");

        //     //// set object's transform
        //     Matrix4f t;
        //     t << 1, 0, 0, -1.5,
        //         0, 1, 0, -1,
        //         0, 0, 1, 0.5,
        //         0, 0, 0, 1;
        //     sphere->Set_Model_Matrix(t);

        //     //// set object's material
        //     sphere->Set_Ka(Vector3f(0.1, 0.1, 0.1));
        //     sphere->Set_Kd(Vector3f(0.7, 0.7, 0.7));
        //     sphere->Set_Ks(Vector3f(2, 2, 2));
        //     sphere->Set_Shininess(128);

        //     //// bind texture to object
        //     sphere->Add_Texture("tex_color", OpenGLTextureLibrary::Get_Texture("sphere_color"));
        //     sphere->Add_Texture("tex_normal", OpenGLTextureLibrary::Get_Texture("sphere_normal"));

        //     //// bind shader to object
        //     sphere->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("basic"));
        // }

        //// Here we load a bunny object with the basic shader to show how to add an object into the scene
        // {
        //     //// create object by reading an obj mesh
        //     auto bunny = Add_Obj_Mesh_Object("obj/bunny.obj");

        //     //// set object's transform
        //     Matrix4f t;
        //     t << 1, 0, 0, 1.5,
        //         0, 1, 0, 0,
        //         0, 0, 1, 0,
        //         0, 0, 0, 1;
        //     Matrix4f scaler;
        //     bunny->Set_Model_Matrix(t);

        //     //// set object's material
        //     bunny->Set_Ka(Vector3f(0.1, 0.1, 0.1));
        //     bunny->Set_Kd(Vector3f(0.7, 0.7, 0.7));
        //     bunny->Set_Ks(Vector3f(2, 2, 2));
        //     bunny->Set_Shininess(128);

        //     //// bind texture to object
        //     bunny->Add_Texture("tex_color", OpenGLTextureLibrary::Get_Texture("bunny_color"));
        //     bunny->Add_Texture("tex_normal", OpenGLTextureLibrary::Get_Texture("bunny_normal"));

        //     //// bind shader to object
        //     bunny->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("basic"));
        // }

        //// Here we show an example of adding a mesh with noise-terrain (A6)
        // {
        //     //// create object by reading an obj mesh
        //     auto terrain = Add_Obj_Mesh_Object("obj/plane.obj");

        //     //// set object's transform
        //     Matrix4f r, s, t;
        //     r << 1, 0, 0, 0,
        //         0, 0.5, 0.67, 0,
        //         0, -0.67, 0.5, 0,
        //         0, 0, 0, 1;
        //     s << 0.5, 0, 0, 0,
        //         0, 0.5, 0, 0,
        //         0, 0, 0.5, 0,
        //         0, 0, 0, 1;
        //     t << 1, 0, 0, -2,
        //          0, 1, 0, 0.5,
        //          0, 0, 1, 0,
        //          0, 0, 0, 1,
        //     terrain->Set_Model_Matrix(t * s * r);

        //     //// set object's material
        //     terrain->Set_Ka(Vector3f(0.1f, 0.1f, 0.1f));
        //     terrain->Set_Kd(Vector3f(0.7f, 0.7f, 0.7f));
        //     terrain->Set_Ks(Vector3f(1, 1, 1));
        //     terrain->Set_Shininess(128.f);

        //     //// bind shader to object (we do not bind texture for this object because we create noise for texture)
        //     terrain->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("terrain"));
        // }
        // {
        //     //// create object by reading an obj mesh
        //     auto bunny = Add_Obj_Mesh_Object("obj/bunny.obj");

        //     //// set object's transform
        //     Matrix4f t;
        //     t << 1, 0, 0, 1.5,
        //         0, 1, 0, 0,
        //         0, 0, 1, 0,
        //         0, 0, 0, 1;
        //     bunny->Set_Model_Matrix(t);

        //     //// set object's material
        //     bunny->Set_Ka(Vector3f(0.1, 0.1, 0.1));
        //     bunny->Set_Kd(Vector3f(0.7, 0.7, 0.7));
        //     bunny->Set_Ks(Vector3f(2, 2, 2));
        //     bunny->Set_Shininess(128);

        //     //// bind texture to object
        //     bunny->Add_Texture("tex_color", OpenGLTextureLibrary::Get_Texture("bunny_color"));
        //     bunny->Add_Texture("tex_normal", OpenGLTextureLibrary::Get_Texture("bunny_normal"));

        //     //// bind shader to object
        //     bunny->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("basic"));
        // }

        //// Here we show an example of adding a mesh with noise-terrain (A6)
        {
            //// create object by reading an obj mesh
            auto terrain = Add_Obj_Mesh_Object("obj/plane.obj");

            //// set object's transform
            Matrix4f r, s, t;
            r << 1, 0, 0, 0,
                0, 0, 1, 0,
                0, -1, 0, 0,
                0, 0, 0, 1;
            s << 12.0, 0, 0, 0,
                0, 12.0, 0, 0,
                0, 0, 12.0, 0,
                0, 0, 0, 1;
            t << 1, 0, 0, -8,
                 0, 1, 0, 0,
                 0, 0, 1, 7,
                 0, 0, 0, 1,
            terrain->Set_Model_Matrix(t * s * r);

            //// set object's material
            terrain->Set_Ka(Vector3f(0.1f, 0.1f, 0.1f));
            terrain->Set_Kd(Vector3f(0.7f, 0.7f, 0.7f));
            terrain->Set_Ks(Vector3f(1, 1, 1));
            terrain->Set_Shininess(128.f);

            //// bind shader to object (we do not bind texture for this object because we create noise for texture)
            terrain->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("terrain"));
        }

        //// Here we show an example of adding a transparent object with alpha blending
        //// This example will be useful if you implement objects such as tree leaves, grass blades, flower pedals, etc.
        //// Alpha blending will be turned on automatically if your texture has the alpha channel
        // {
        //     //// create object by reading an obj mesh
        //     auto sqad = Add_Obj_Mesh_Object("obj/sqad.obj");

        //     //// set object's transform
        //     Matrix4f t;
        //     t << 1, 0, 0, -0.5,
        //         0, 1, 0, 0,
        //         0, 0, 1, 1.5,
        //         0, 0, 0, 1;
        //     sqad->Set_Model_Matrix(t);

        //     //// bind texture to object
        //     sqad->Add_Texture("tex_color", OpenGLTextureLibrary::Get_Texture("window_color"));

        //     //// bind shader to object
        //     sqad->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("blend"));
        // }

        //// Here we show an example of adding a billboard particle with a star shape using alpha blending
        //// The billboard is rendered with its texture and is always facing the camera.
        //// This example will be useful if you plan to implement a CPU-based particle system.
        // {
        //     //// create object by reading an obj mesh
        //     auto sqad = Add_Obj_Mesh_Object("obj/sqad.obj");

        //     //// set object's transform
        //     Matrix4f t;
        //     t << 1, 0, 0, 0,
        //          0, 1, 0, 0,
        //          0, 0, 1, 2.5,
        //          0, 0, 0, 1;
        //     sqad->Set_Model_Matrix(t);

        //     //// bind texture to object
        //     sqad->Add_Texture("tex_color", OpenGLTextureLibrary::Get_Texture("star_color"));

        //     //// bind shader to object
        //     sqad->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("billboard"));
        // }

        /* Load and render fire place */
        {
            //// create object by reading an obj mesh
            srand(time(0)); // Seed with the current time



            for (int i = 0; i < snowNum; i++) {

                int randomNumberX = std::rand() % 15 - 7;
                int randomNumberY = 1 + std::rand() % (5 - 1 + 1); 
                int randomNumberZ = std::rand() % 19 - 15;
                


                auto sqad = Add_Obj_Mesh_Object("obj/sqad.obj");
                sqad->name = "Snow";
                sqad->Get_Model_Matrix();
                // std::cout << randomNumber << std::endl;

                //// set object's transform

                Matrix4f t;
                t << 1, 0, 0, randomNumberX,
                    0, 1, 0, randomNumberY,
                    0, 0, 1, randomNumberZ,
                    0, 0, 0, 1;
                sqad->Set_Model_Matrix(t);

                //// bind texture to object
                // sqad->Add_Texture("tex_color", OpenGLTextureLibrary::Get_Texture("star_color"));


                sqad->Add_Texture("tex_color", OpenGLTextureLibrary::Get_Texture("snow_color"));


                //// bind shader to object
                sqad->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("billboard"));
            }

           
        }

        /* Campfire */
        {
            auto campfire = Add_Obj_Mesh_Object("obj/campfire.OBJ");

            /* set object's transform */
            Matrix4f t;
            t << 0.02f, 0, 0, 1.0f,
                0, 0.02f, 0, 0,
                0, 0, 0.02f, -3.0f,
                0, 0, 0, 1;
            campfire->Set_Model_Matrix(t);

            /* bind texture to object */
            campfire->Add_Texture("tex_color", OpenGLTextureLibrary::Get_Texture("campfire_color"));
            campfire->Add_Texture("tex_normal", OpenGLTextureLibrary::Get_Texture("campfire_normal"));

            /* bind shader to object */
            campfire->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("basic"));
        }

        /* Add tent to scene */
        {
            auto tent = Add_Obj_Mesh_Object("obj/tent.OBJ");

            /* set object's transform */
            Matrix4f t, r;
            t << 1, 0, 0, 5.0f,
                0, 1, 0, 0,
                0, 0, 1, -8.0f,
                0, 0, 0, 1;
            r << cos(PI / 4), 0, -sin(PI / 4), 0,
                0, 1, 0, 0,
                sin(PI / 4), 0, cos(PI / 4), 0,
                0, 0, 0, 1;
            tent->Set_Model_Matrix(t * r);

            /* bind texture to object */
            tent->Add_Texture("tex_color", OpenGLTextureLibrary::Get_Texture("tent_color"));
            tent->Add_Texture("tex_normal", OpenGLTextureLibrary::Get_Texture("tent_normal"));

            /* bind shader to object */
            tent->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("basic"));
        }

        /* Flame on campfire */
        // {
        //     /* make three flame objects */
        //     for (int i = 0; i < 3; ++i)
        //     {
        //         /* create object by reading an obj mesh */
        //         auto sqad = Add_Obj_Mesh_Object("obj/sqad.obj");

        //         /* create object's transform */
        //         Matrix4f t, s, r;
        //         t << 1.0f, 0.0f, 0.0f, -2.5f,
        //             0.0f, 1.0f, 0.0f, 0.0f,
        //             0.0f, 0.0f, 1.0f, -2.5f,
        //             0.0f, 0.0f, 0.0f, 1.0f;
        //         s << 1.2f, 0.0f, 0.0f, 0.0f,
        //             0.0f, 1.2f, 0.0f, 0.0f,
        //             0.0f, 0.0f, 1.2f, 0.0f,
        //             0.0f, 0.0f, 0.0f, 1.0f;
        //         r << cos(i * 2 * PI / 3), 0.0f, sin(i * 2 * PI / 3), 0.0f,
        //             0.0f, 1.0f, 0.0f, 0.0f,
        //             -sin(i * 2 * PI / 3), 0.0f, cos(i * 2 * PI / 3), 0.0f,
        //             0.0f, 0.0f, 0.0f, 1.0f;

        //         /* set model matrix */
        //         sqad->Set_Model_Matrix(t * s * r);

        //         /* bind texture to object */
        //         sqad->Add_Texture("tex_color", OpenGLTextureLibrary::Get_Texture("flame_color"));

        //         /* bind shader to object */
        //         sqad->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("blend"));
        //     }
        // } 

        /* Fire particles */
        {

            //// create object's transform
            Matrix4f t;
            t << 0.1f, 0.0f, 0.0f, 1.0f,
                0.0f, 0.1f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.1f, -3.0f,
                0.0f, 0.0f, 0.0f, 1.0f;

            /* Define random number generator */
            std::random_device rd;
            std::default_random_engine generator(rd());
            std::uniform_real_distribution<double> distribution(-1.0, 1.0);
            /* create 8 fire particles */
            for (int i = 0; i < NUM_FIRE_PARTICLES; ++i)
            {
                //// create object by reading an obj mesh
                auto sqad = Add_Obj_Mesh_Object("obj/sqad.obj");

                //// create vector multipliers

                std::vector<double> vel;
                vel.push_back(distribution(generator));
                vel.push_back(distribution(generator) * 0.7 + 0.9);
                vel.push_back(distribution(generator));

                //// set model matrix
                sqad->Set_Model_Matrix(t);

                //// bind texture to object
                sqad->Add_Texture("tex_color", OpenGLTextureLibrary::Get_Texture("fire_color"));

                //// bind shader to object
                sqad->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("billboard"));

                /* Calculate time offset */
                clock_t time = startTime + i * (2000 / NUM_FIRE_PARTICLES);
                // std::cout << time << std::endl;

                //// instantiate fire object
                Fire curr(sqad, time, vel, t);

                //// add particle to fire particles vector
                fireParticles.push_back(curr);
            }
        }

        

        //// Here we show an example of shading (ray-tracing) a sphere with environment mapping
        /*
        {
            //// create object by reading an obj mesh
            auto sphere2 = Add_Obj_Mesh_Object("obj/sphere.obj");

            //// set object's transform
            Matrix4f t;
            t << .6, 0, 0, 0,
                0, .6, 0, -.5,
                0, 0, .6, 1,
                0, 0, 0, 1;
            sphere2->Set_Model_Matrix(t);

            //// bind shader to object
            sphere2->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("environment")); // bind shader to object
        }
        */

        //// Here we create a mesh object with two triangle specified using a vertex array and a triangle array.
        //// This is an example showing how to create a mesh object without reading an .obj file. 
        //// If you are creating your own L-system, you may use this function to visualize your mesh.
        // {
        //     std::vector<Vector3> vertices = { Vector3(0.5, 0, 0), Vector3(1, 0, 0), Vector3(1, 1, 0), Vector3(0, 1, 0) };
        //     std::vector<Vector3i> elements = { Vector3i(0, 1, 2), Vector3i(0, 2, 3) };
        //     auto obj = Add_Tri_Mesh_Object(vertices, elements);
        //     // ! you can also set uvs 
        //     obj->mesh.Uvs() = { Vector2(0, 0), Vector2(1, 0), Vector2(1, 1), Vector2(0, 1) };

        //     Matrix4f t;
        //     t << 1, 0, 0, -0.5,
        //         0, 1, 0, -1.5,
        //         0, 0, 1, 0,
        //         0, 0, 0, 1;

        //     obj->Set_Model_Matrix(t);

        //     obj->Add_Texture("tex_color", OpenGLTextureLibrary::Get_Texture("buzz_color"));

        //     obj->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("basic"));
        // }

        //// This for-loop updates the rendering model for each object on the list
        for (auto &mesh_obj : mesh_object_array){
            Set_Polygon_Mode(mesh_obj, PolygonMode::Fill);
            Set_Shading_Mode(mesh_obj, ShadingMode::TexAlpha);
            mesh_obj->Set_Data_Refreshed();
            mesh_obj->Initialize();
        }
        Toggle_Play();
    }

    //// add mesh object by reading an .obj file
    OpenGLTriangleMesh *Add_Obj_Mesh_Object(std::string obj_file_name)
    {
        auto mesh_obj = Add_Interactive_Object<OpenGLTriangleMesh>();
        Array<std::shared_ptr<TriangleMesh<3>>> meshes;
        // Obj::Read_From_Obj_File(obj_file_name, meshes);
        Obj::Read_From_Obj_File_Discrete_Triangles(obj_file_name, meshes);

        mesh_obj->mesh = *meshes[0];
        std::cout << "load tri_mesh from obj file, #vtx: " << mesh_obj->mesh.Vertices().size() << ", #ele: " << mesh_obj->mesh.Elements().size() << std::endl;

        mesh_object_array.push_back(mesh_obj);
        return mesh_obj;
    }

    //// add mesh object by reading an array of vertices and an array of elements
    OpenGLTriangleMesh* Add_Tri_Mesh_Object(const std::vector<Vector3>& vertices, const std::vector<Vector3i>& elements)
    {
        auto obj = Add_Interactive_Object<OpenGLTriangleMesh>();
        mesh_object_array.push_back(obj);
        // set up vertices and elements
        obj->mesh.Vertices() = vertices;
        obj->mesh.Elements() = elements;

        return obj;
    }

    //// Go to next frame
    virtual void Toggle_Next_Frame()
    {
        GLfloat timeElapsed = GLfloat(clock() - startTime) / CLOCKS_PER_SEC;
        for (auto &mesh_obj : mesh_object_array)
            mesh_obj->setTime(GLfloat(clock() - startTime) / CLOCKS_PER_SEC);
        for (auto &mesh_obj : mesh_object_array)
        {
            if (mesh_obj->name == "Snow") 
            {
                glm::mat4 currentTransform = mesh_obj->Get_Model_Matrix();
                int depth = currentTransform[3][2];
                currentTransform[3][1] -= 0.025f; 
                if (depth >= -6) {
                    currentTransform[3][1] -= 0.026f; 
                }
                if (depth >= -1) {
                    currentTransform[3][1] -= 0.028f; 
                }

                float frequency = 0.05f;        
                float amplitude = 0.08f;        
                currentTransform[3][0] += amplitude * sin(frequency * 5 + (depth / 2)); 

                // float frequency = 0.05f;        
                // float amplitude = 0.08f;        
                currentTransform[3][2] += .02 * cos(frequency / 10); 

                if (currentTransform[3][1] < -2.5 || currentTransform[3][0] > 10 || currentTransform[3][0] < -10)  
                {
                    currentTransform[3][1] = 5.0f;  
                    currentTransform[3][0] = std::rand() % 15 - 7;  
                    currentTransform[3][2] = std::rand() % 19 - 15;
                }
                Eigen::Matrix<float, 4, 4> eigenMatrix;
                for (int i = 0; i < 4; i++) {
                    for (int j = 0; j < 4; j++) {
                        eigenMatrix(i, j) = currentTransform[j][i];  
                    }
                }
                mesh_obj->Set_Model_Matrix(eigenMatrix);
            }
        }
        







        if (bgEffect){
            bgEffect->setResolution((float)Win_Width(), (float)Win_Height());
            bgEffect->setTime(GLfloat(clock() - startTime) / CLOCKS_PER_SEC);
            bgEffect->setFrame(frame++);
        }

        if (skybox){
            skybox->setTime(GLfloat(clock() - startTime) / CLOCKS_PER_SEC);
        }

        for (auto& fire : fireParticles)
        {

            // Get time passed since last reset
            double t = ((double) clock() - fire.getTime()) / CLOCKS_PER_SEC;

            // std::cout << "Time passed since last reset: " << t << " seconds." << std::endl;
            // std::vector<double> vel = *(fire)->initVel;
            // std::cout << "Time since last reset: " << t << std::endl;
            // std::cout << fire.getTransform() << std::endl;
            // std::cout << t << std::endl;
            // std::cout << clock() << std::endl;
            
            // If 2 seconds have passed since last reset, reset fire's transform to original transform and reset t to now
            if (t >= 2.0)
            {
                fire.getObj()->Set_Model_Matrix(fire.getTransform());
                fire.setTime(clock());
                
                // std::cout << fire->time << std::endl;
            }
            else if (t >= 0.0)
            {
                // Calculate x position
                double x = sin((fire.getVel()[0] + t) * PI);

                // Calculate z position
                double z = cos((fire.getVel()[2] + t) * PI);
                
                // Make translation matrix
                Matrix4f newTransform;
                newTransform << 0.0f, 0.0f, 0.0f, x * fire.getVel()[0],
                    0.0f, 0.0f, 0.0f, t * fire.getVel()[1],
                    0.0f, 0.0f, 0.0f, z * fire.getVel()[2],
                    0.0f, 0.0f, 0.0f, 0.0f;
                
                // Set fire's transform matrix
                fire.getObj()->Set_Model_Matrix(newTransform + fire.getTransform());
            }
        }

        OpenGLViewer::Toggle_Next_Frame();
    }

    virtual void Run()
    {
        OpenGLViewer::Run();
    }
};

int main(int argc, char *argv[])
{
    MyDriver driver;
    driver.Initialize();
    driver.Run();
}

#endif