#version 330 core

uniform vec2 iResolution;       /* window resolution */
uniform float iTime;            /* time */
uniform int iFrame;             /* frame index */
in vec2 fragCoord;              /* screen space coordinate */
out vec4 outputColor;           /* output color */

/////////////////////////////////////////////////////
//// Macro definition for time: this is the actual time you use for your animation
/////////////////////////////////////////////////////
//// You may adjust the multipler below to make the animaiton run fast (by increasing the value) 
//// or run slower (by decreasing the value), according to your needs (it varies on different laptops).
//// We set the default value to be 1.0.
/////////////////////////////////////////////////////

#define Time (iTime*1.0)            

#define PI 3.14159265359
#define TWO_PI 6.28318530718
#define Gravity 0.7             /* gravity */
#define NUM_STAR 30.            /* number of stars on the sky */
#define NUM_EMISSION 30.        /* number of emission particles */
#define NUM_FIREWORKS 175         /* number of fireworks */
#define DURATION 25.             /* duration of each fireworks period */

const vec2 g = vec2(.0, -Gravity); /* gravity */

/////////////////////////////////////////////////////
//// Hash functions
/////////////////////////////////////////////////////

//// This hash function takes input t and returns random float between 0 and 1
float hash1d(float t)
{
    t += 1.;
    return fract(sin(t * 674.3) * 453.2);
}

float hash1dOther(float t, float i)
{
    t += 1.0; // Offset
    return fract(sin(t * 674.3 + i * 123.4) * 453.2);
}






//// This hash function takes input t and returns random vec2 with each component between 0 and 1
vec2 hash2d(float t)
{
    t += 1.;
    float x = fract(sin(t * 674.3) * 453.2);
    float y = fract(sin((t + x) * 714.3) * 263.2);

    return vec2(x, y);
}

//// This hash function takes input t and returns random vec3 with each component between 0 and 1
vec3 hash3d(float t)
{
    t += 1.;
    float x = fract(sin(t * 674.3) * 453.2);
    float y = fract(sin((t + x) * 714.3) * 263.2);
    float z = fract(sin((t + y) * 134.3) * 534.2);

    return vec3(x, y, z);
}

//// This hash function takes input t and returns a random vec2 on a circle
vec2 hash2d_polar(float t)
{
    t += 1.;
    float a = fract(sin(t * 674.3) * 453.2) * TWO_PI;
    float d = fract(sin((t + a) * 714.3) * 263.2);
    return vec2(sin(a), cos(a)) * d;
}



vec3 hash3d_color(float time) {
    time = time * 100;
    float r = sin(time); 
    float g = sin(time + 2.094); 
    float b = sin(time + 4.188); 
    return vec3(r,g,b);
}











/////////////////////////////////////////////////////
//// Step 1: render a single particle
//// In this function, you are asked to implement the rendering of a single particle onto the screen.
//// The task is to calculate the distance between the current fragment and the particle (both in 2D), 
//// and then use the distance to build a decay function f(d)=1/d, and multiply the function value with brightness and color
//// to calculate the fragment color value.
/////////////////////////////////////////////////////

vec3 renderParticle(vec2 fragPos, vec2 particlePos, float brightness, vec3 color)
{
    vec3 fragColor = vec3(0.0);

	/* your implementation starts */
    float x = (fragPos.x - particlePos.x) * (fragPos.x - particlePos.x);
    float y = (fragPos.y - particlePos.y) * (fragPos.y - particlePos.y);
    float dis = sqrt(x + y);



    fragColor = color * (1.0 / (dis * 10)) * brightness;
    /* your implementation ends */

    return fragColor;
}

/////////////////////////////////////////////////////
//// Step 2: render the starry sky with multiple particles
//// In this function, you are asked to implement the rendering of a starry sky with multiple particles.
//// Your tasks include three parts within the for-loop that traverses all the stars: 
//// (1) produce a time-varying brightness by using the variable t and the default value (0.0004).
//// (2) come up with a color for each star (it can be random or uniform, up to your preference);
//// (3) call the renderParticle function you've implemented in the previous steps with the appropriate parameters
//// and accumulate the result to fragColor.
//// After implementing this function, uncomment the Step 2 block in mainImage() to testify its correctness.
//// You should be able to see a starry sky with blinking stars if everything is implemented correctly.
/////////////////////////////////////////////////////

// so basically just put a bunch of particles in random positions
vec3 renderStars(vec2 fragPos)
{
    vec3 fragColor = vec3(0.01, 0.04, 0.3);
    float t = Time;

    for(float i = 0.; i < NUM_STAR; i++){
        vec2 pos = (hash2d(i) - .5) * iResolution.xy / iResolution.y;        
        float brightness = .0004;

        /* your implementation starts */
        vec3 star_color = vec3(1, 1, 1);
        // brightness = sin(t * 3.0 + i); // Periodic blinking

        fragColor += renderParticle(fragPos, pos, brightness * hash1d(t), star_color);
        /* your implementation ends */
    }

    return fragColor;
}

/////////////////////////////////////////////////////
//// Step 3A: simulate the motion of a single particle by programming ballistic motion
//// In this function, you are asked to calculate the position of the particle using the expression of ballistic motion.
//// The function takes the initial position, initial velocity, and time t as input, and returns the particle's current location.
/////////////////////////////////////////////////////

vec2 moveParticle(vec2 initPos, vec2 initVel, float t)
{
    float terminalVelocity = -0.15;  
    float acceleration = -0.01;      


    // float randVal = random(Time);

    // float minWindStrength = 0.1;
    // float maxWindStrength = 0.5;
    // float windStrength = mix(minWindStrength, maxWindStrength, randVal); 
    float windStrength = 0.1;


    float windFrequency = .2;      
    
    float randomWindOffset = sin(initPos.x * 10.0 + iTime * windFrequency) * windStrength;
    float randomFallSpeed = initVel.y + acceleration * t;

    randomFallSpeed = max(randomFallSpeed, terminalVelocity);

    vec2 currentPos = initPos + vec2(randomWindOffset, randomFallSpeed * t);

    return currentPos;
}






/////////////////////////////////////////////////////
//// Step 3B: putting simulation and rendering together in one function call
//// In this function, you will practice to combine the animaiton and rendering functions together 
//// by calling moveParticle() and renderParticle() you have implemented to calculate the fragment color.
//// The idea is to update the particle's current position with moveParticle() first, 
//// and then use this position as an input for renderParticles() to calculate the fragment color.
//// After implementing both Step 3A and 3B, you want to testify its correctness by uncommenting Step 3 in mainImage(). 
//// The expected result is the animation of a single particle that moves along a ballistic trajectory.
/////////////////////////////////////////////////////

vec3 simSingleParticle(vec2 fragPos, vec2 initPos, vec2 initVel, float t, float brightness, vec3 color)
{
    vec3 fragColor = vec3(0.0);

    /* your implementation starts */
    vec2 pos = moveParticle(initPos, initVel, t);
    fragColor += renderParticle(fragPos, pos, brightness, color);
    /* your implementation ends */

    return fragColor;
}

/////////////////////////////////////////////////////
//// Step 4: simulating fireworks
//// You will implement the animation of a fireworks explosion in this function.
//// The key idea is to separate the animation into two phases.
//// For Phase I, we animate a single "boss particle" that follows a ballistic trajectory.
//// For Phase II, we spawn a number of emitting particles based on the position of the boss particle at the emitTime,
//// and then simulate the trajectory of each emitting particle starting from the emitPos, with a random emitVel, using emitT. 
//// The simulation is implemented by calling the function simSingleParticle you have implemented from the previous step.
//// The color returned from the function call needs to be accumulated to fragColor that will be returned as the fragment color.
//// Your implementation will be focused on the Phase II part, 
//// in which you need to update the emitting particle's brightness to show some flickering and fading effects, 
//// and call simSingleParticle() with appropriate parameters to accumulate its color to the fragment.
//// After implementing this step, you can test the fireworks effect by uncommenting the block of Step 4 in mainImage().
/////////////////////////////////////////////////////

vec3 simSingleFirework(vec2 fragPos, vec2 launchPos, vec2 launchVel, float t, vec3 color, float brightness)
{
    vec3 fragColor = vec3(0.0);
    float emitTime = 1.5;

    //NOTE TO SELF: EMIT TIME SETS THE LIFETIME OF THE PARTICLE
    // if(t < emitTime){
        // float brightness = .002;



        vec2 initPos = launchPos;
        vec2 initVel = launchVel;
        fragColor += simSingleParticle(fragPos, initPos, initVel, t, brightness, color);
    // }



    // else{
    //     float emitT = t - emitTime; // time since emission
    //     vec2 emitPos = moveParticle(launchPos, launchVel, emitTime);
    //     float brightness = .002 * emitT;

    //     for(float i = 0.; i < NUM_EMISSION; i++){
    //         vec2 emitVel = hash2d_polar(i) * .7; // random direction with max magnitude 0.7

    //         /* your implementation starts */
    //         // fragColor += simSingleParticle(fragPos, emitPos, emitVel, emitT, brightness, color);
    //         fragColor += simSingleParticle(fragPos, emitPos, emitVel, emitT, brightness, hash3d(Time));    

    //         /* your implementation ends */
    //     }
    // }

    return fragColor;
}



float noise(vec2 p) {
	return fract(sin(dot(p, vec2(10.32562, 69.2345))) * 23495.534);

}

vec3 renderSnow(vec2 fragPos)
{
    vec3 fragColor = vec3(0.0);

    for(float i = 0.; i < NUM_FIREWORKS; i++){
        float lauchTime = i;
        float relTime = Time - lauchTime;
        float t = mod(relTime, DURATION) / 1.1;
        // float t = lauchTime * 2;

        // float idx = floor(relTime / DURATION);

        // vec2 launchPos = vec2((hash1d(i) - .5) * iResolution.x / iResolution.y, .8); 

        vec2 launchPos = vec2(noise(vec2(i, 0.0)) *  iResolution.x / iResolution.y, noise(vec2(.7, 0.0)));




        if (mod(i,3.0) == 0) {
            launchPos = vec2((hash1dOther(i, i/ .3) - .5) * iResolution.x / iResolution.y, .8); 
        } else if (mod(i,4.0) == 0){
            launchPos = vec2((hash1dOther(i, i/ 2.5) - .5) * iResolution.x / iResolution.y, .8); 
        } else if (mod(i,5.0) == 0){
            launchPos= vec2((hash1dOther(i, i/ 1.4) - .5) * iResolution.x / iResolution.y, .8); 
        } else if (mod(i, 6.0) == 0){
            launchPos= vec2((hash1dOther(i, i/ 4) - .5) * iResolution.x / iResolution.y, .8); 
        }
        
        
        
        vec2 launchVel = vec2(0, -1) / 2;
        // vec3 color = sin(40. * hash3d(lauchTime) * idx) * 0.25 + 0.75;
        // vec3 color = vec3(1, 0 ,0);
        float brightness = 0.008;
        if (mod(i, 4.0) == 0) {
            brightness = mix(0.5, 1.0, hash1d(i)) * 0.002;
            t /= 1.8;
            // to to self updating t changes the acceleration and slows it down a lot!
        }
        
        if (mod(i, 7.0) == 0) {
            brightness = mix(0.5, 1.0, hash1d(i)) * 0.005;
            t /= 1.3;
        }

        if (mod(i, 10.0) == 0) {
            brightness = mix(0.5, 1.0, hash1d(i)) * 0.003;
            t /= 2;
        } 



        // if (mod(i, 5.0) == 0) {
        //     brightness = mix(0.5, 1.0, hash1d(i)) * 0.007;
        //     t /= 1.2;
        // } 

        vec3 color = vec3(1.f, 1.f, 1.f);
        fragColor += simSingleFirework(fragPos, launchPos, launchVel, t, color, brightness);
    }

    return fragColor;
}

void mainImage(out vec4 outputColor, in vec2 fragCoord)
{
    //// fragPos's center is at the center of the screen, fragPos.y range is [-0.5, 0.5]
    vec2 fragPos = (fragCoord - .5 * iResolution.xy) / iResolution.y;

    vec3 fragColor = vec3(0.0);

    //// Step 1: render single particle
    // {
    //     vec2 pos = vec2(0., 0.);
    //     float brightness = 0.005;
    //     vec3 color = vec3(1.f, 1.f, 1.f);
    //     fragColor = renderParticle(fragPos, pos, brightness, color);
    // }

    // Step 2: render starry sky
    // Uncomment the following block to test your Step 2 implementation
    // {
    //    fragColor = renderStars(fragPos);
    // }

    //// Step 3: simulate single particle
    //// Uncomment the following block to test your Step 3 implementation
    // {
    //    vec2 initPos = vec2(0, 0);
    //    vec2 initVel = vec2(0, 0);
    //    float t = mod(Time, DURATION);
    //    float brightnes = .005;
    //    vec3 color = vec3(1.f, 1.f, 1.f);
    //    fragColor = simSingleParticle(fragPos, initPos, initVel, t, brightnes, color);
    // }
    
    //// Step 4: simulate fireworks
    //// Uncomment the following block to test your Step 4 implementation
    {
       fragColor = renderSnow(fragPos);
    }
    // float alpha = (fragColor.r + fragColor.g + fragColor.b > 0.01) ? 1.0 : 0.0;
    outputColor = vec4(fragColor, 1);
}

void main()
{
    mainImage(outputColor, fragCoord);
}
