#include <stdio.h>
#include "raylib/raylib.h"
#include <math.h>

struct Vec2
{
    float x;
    float y;
};

struct CenterVec
{
    float x;
    float y;
};

struct ParticleSize
{
    int x;
    int y;
};

struct Vec2vel
{
    float x_velocity;
    float y_velocity;
};

struct Rad
{
    float radius;
};

// Particle with position, velocity, and size
struct Particle 
{
    struct Vec2 pos;             // x and y of circle particle, will be the same value for circles.
    struct Vec2vel vel;          // velocity
    struct ParticleSize size;    // size of particle
    Color color;                 // color of particle
    struct Rad rad;              // radius of particle
    float gravity;               // particles gravity
};

int particle_count;
struct Particle particle_array[1024];

int main(){
    InitWindow(800, 600, "Particles");
    HideCursor();

    // Keeps track of when previous particle spawned and limits when a new one can spawn again.
    float previousParticleSpawn = 0.0f;
    float spawnCooldown = 0.2f;

    while(!WindowShouldClose())
    {

        float time = GetTime();
        ClearBackground(BLACK);
        BeginDrawing();

        Vector2 mousePos = GetMousePosition();

        // Draw circle at mouse
        DrawCircle(mousePos.x, mousePos.y, 5, WHITE);

        // Spawn particle on left-click
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && particle_count < 1024)
        {
            // Checks if new ball can spawn yet
            if (time - previousParticleSpawn >= spawnCooldown)
            {
            struct Particle p;
            p.pos.x = mousePos.x;
            p.pos.y = mousePos.y;
            p.vel.x_velocity = GetRandomValue(-5, 5);
            p.vel.y_velocity = GetRandomValue(-5, 5);
            p.size.x = 15;  // Randomizes ball size between two values. Then give x value to y so it's the same size.
            p.size.y = 15;
            p.rad.radius = p.size.y * 0.5f;
            //p.gravity = GetRandomValue(1, 10) * 0.01f;
            p.color = WHITE;
            p.color.a = 255; // 100% alpha, decrease number to lower transparensy.
            previousParticleSpawn = time;

            particle_array[particle_count] = p;
            particle_count++;

            printf("Number of particles: %d\n", particle_count);
            printf("Time spaned: %f\n", time);
            }
        }

        // Update and draw all particles in array
        for(int i = 0; i < particle_count; i++)
        {
            // Apply gravity
            //particle_array[i].vel.y_velocity += particle_array[i].gravity;
            particle_array[i].pos.y += particle_array[i].vel.y_velocity;
            particle_array[i].pos.x += particle_array[i].vel.x_velocity;

            for(int c = i + 1; c < particle_count; c++)
            {
                if (CheckCollisionCircles(
                    (Vector2){ particle_array[i].pos.x, particle_array[i].pos.y }, particle_array[i].rad.radius,
                    (Vector2){ particle_array[c].pos.x, particle_array[c].pos.y }, particle_array[c].rad.radius))
                {
                    // Find the difference in position between two particles. dx and dy is DELTA.
                    float dx = particle_array[i].pos.x - particle_array[c].pos.x;
                    float dy = particle_array[i].pos.y - particle_array[c].pos.y;

                    // Calculate how far apart the particles are at collision
                    float distance = sqrtf(dx*dx + dy*dy);

                    // We stop dividing by 0 if particles at right on top of each other because we can't calculate the normal if we divide by 0.
                    if (distance == 0.0f) {
                        dx = 0.001f; // Move a small amount in x direction if particles are on top of each other.
                        dy = 0.0f;
                        distance = 0.001f; // new distance from eachothers center.
                    }

                    // Make a normal vector. A normal is a line between the center of two particles.
                    float normalX = dx / distance;
                    float normalY = dy / distance;

                    // Move particles if they are overlapping to prevent getting stuck in each other.
                    float combinedRadius = particle_array[i].rad.radius + particle_array[c].rad.radius;
                    float overlap = combinedRadius - distance;
                    if (overlap > 0.0f) {
                        float move = overlap * 0.5f;  // move each particle half the overlap
                        particle_array[i].pos.x += normalX * move;
                        particle_array[i].pos.y += normalY * move;
                        particle_array[c].pos.x -= normalX * move;
                        particle_array[c].pos.y -= normalY * move;
                    }

                    // Get current velocities for x and y.
                    float vel_i_x = particle_array[i].vel.x_velocity;
                    float vel_i_y = particle_array[i].vel.y_velocity;
                    float vel_c_x = particle_array[c].vel.x_velocity;
                    float vel_c_y = particle_array[c].vel.y_velocity;

                    // Calculate how fast each particle is moving along the normal.
                    float vel_i_normal = vel_i_x * normalX + vel_i_y * normalY;
                    float vel_c_normal = vel_c_x * normalX + vel_c_y * normalY;

                    // If particles are moving away from eachother, don't change velocity.
                    float relativeVelocity = vel_i_normal - vel_c_normal;
                    if (relativeVelocity > 0.0f) {
                        // Particles are separating so we do not change speed.
                    } else {
                        // Make particles bounce by swapping normal speed.
                        float newVel_i_normal = vel_c_normal;
                        float newVel_c_normal = vel_i_normal;

                        // The tangent speed does not change.
                        float tangentX = -normalY;
                        float tangentY = normalX;

                        float vel_i_tangent = vel_i_x * tangentX + vel_i_y * tangentY;
                        float vel_c_tangent = vel_c_x * tangentX + vel_c_y * tangentY;

                        // Combine normal and tangent to get final x and y speed to finish the collision bounce.
                        particle_array[i].vel.x_velocity = newVel_i_normal * normalX + vel_i_tangent * tangentX;
                        particle_array[i].vel.y_velocity = newVel_i_normal * normalY + vel_i_tangent * tangentY;

                        particle_array[c].vel.x_velocity = newVel_c_normal * normalX + vel_c_tangent * tangentX;
                        particle_array[c].vel.y_velocity = newVel_c_normal * normalY + vel_c_tangent * tangentY;
                    }

                }
            } 
            
            // Make balls stop at edges of screen and bounce
            // Change *0.8 value to increase or decrease bounce force.
            if (particle_array[i].pos.y + particle_array[i].rad.radius > 600)
                {
                    particle_array[i].pos.y = 600 - particle_array[i].rad.radius;
                    particle_array[i].vel.y_velocity = -particle_array[i].vel.y_velocity * 0.8;
                }
            if (particle_array[i].pos.y - particle_array[i].rad.radius < 0)
                {
                    particle_array[i].pos.y = 0 + particle_array[i].rad.radius;
                    particle_array[i].vel.y_velocity = -particle_array[i].vel.y_velocity * 0.8;
                }
            if (particle_array[i].pos.x + particle_array[i].rad.radius > 800)
                {
                    particle_array[i].pos.x = 800 - particle_array[i].rad.radius;
                    particle_array[i].vel.x_velocity = -particle_array[i].vel.x_velocity * 0.8;
                }
            if (particle_array[i].pos.x - particle_array[i].rad.radius < 0)
                {
                    particle_array[i].pos.x = 0 + particle_array[i].rad.radius;
                    particle_array[i].vel.x_velocity = -particle_array[i].vel.x_velocity * 0.8;
                }

            // Draw particle
            DrawCircle(
                particle_array[i].pos.x, 
                particle_array[i].pos.y, 
                particle_array[i].rad.radius, 
                particle_array[i].color
            );
        }

        DrawFPS(16, 16);
        DrawText(TextFormat("Time: %f", time), 600, 16, 20, WHITE);
        EndDrawing();
    }
}
