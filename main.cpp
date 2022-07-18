/*
    This code was produced by Frost Edson on 06/22/2022 3:07AM
    Contributions from One Lone Coder on the First person Shooting console game
    It is designed to run on windows os due to the screen buffers used that has to work with the "windows.h" library
    This library makes api calls to the windosw screen hence not possible to be used in linux
*/

#include <iostream>
#include <windows.h>
#include <vector>
#include <math.h>
#include <bits/stdc++.h>
#include <chrono>

using namespace std;

int nScreenwidth = 120;
int nScreenheight = 40;

float fPlayerX = 8.0f;  //x_position
float fPlayerY = 8.0f;  //y_postion
float fPlayerA = 0.0f;  //player_angle

int nMapHeight = 16;
int nMapWidth = 16;

float fFOV = 3.14159 / 4.0;   //Field of view
float fDepth = 16.0f;


int main()
{
   //Create screen buffer
   wchar_t *screen = new wchar_t[nScreenwidth*nScreenheight];
   HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
   SetConsoleActiveScreenBuffer(hConsole);
   DWORD dwBytesWritten = 0;

   wstring map;
   map += L"################";
   map += L"#..............#";
   map += L"#..............#";
   map += L"#...####...##..#";
   map += L"#..##..........#";
   map += L"#..##..........#";
   map += L"#.......####...#";
   map += L"#.###...#......#";
   map += L"#.......#......#";
   map += L"#....####......#";
   map += L"#..............#";
   map += L"#...####.......#";
   map += L"#...#..#########";
   map += L"#...#..........#";
   map += L"#..............#";
   map += L"################";

   auto tp1 = chrono::system_clock::now();
   auto tp2 = chrono::system_clock::now();

   //Game loop
   while (1)
   {
       //calculate elapsed time betweena frames
       tp2 = chrono::system_clock::now();
       chrono::duration<float> elapsedTime = tp2 - tp1;
       tp1 = tp2;
       float fElapsedTime = elapsedTime.count();

       //controls
       //Handle CCW rotation
       if(GetAsyncKeyState((unsigned short)'A') & 0x8000)
            fPlayerA -= (0.8f) * fElapsedTime;

       if(GetAsyncKeyState((unsigned short)'D') & 0x8000)
            fPlayerA += (0.8f) * fElapsedTime;

        //Forward backward
       if(GetAsyncKeyState((unsigned short)'W') & 0x8000)
        {
            fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
            fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;

            //collision detect
            if(map[(int)fPlayerY * nMapWidth + (int)fPlayerX] ==  '#'){
                fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
                fPlayerX -= cosf(fPlayerA) * 5.0f * fElapsedTime;
            }
        }

          //Backward backward
       if(GetAsyncKeyState((unsigned short)'S') & 0x8000)
        {
            fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
            fPlayerX -= cosf(fPlayerA) * 5.0f * fElapsedTime;

            //collision detect
            if(map[(int)fPlayerY * nMapWidth + (int)fPlayerX] ==  '#'){
                fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
                fPlayerX += cosf(fPlayerA) * 5.0f * fElapsedTime;
            }
        }

        for(int x = 0; x < nScreenwidth; x++)
        {
            //for each column, calculate the projected ray angle into world space
            float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenwidth) * fFOV;

            float FDistanceToWall = 0;
            bool bHitWall = false;
            bool bBoundary = false;

            float fEyeX = sinf(fRayAngle); //unit vector for ray in player space
            float fEyeY = cosf(fRayAngle);

            while(!bHitWall && FDistanceToWall < fDepth)
            {
                FDistanceToWall += 0.1f;

                int nTestX = (int)(fPlayerX + fEyeX * FDistanceToWall);
                int nTestY = (int)(fPlayerY + fEyeY * FDistanceToWall);

                //Test if ray is out of bounds
                if (nTestX < 0 || nTestY >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight)
                {
                    bHitWall = true;    //just set distance to maximum depth;
                    FDistanceToWall = fDepth;
                } else {
                    //Ray is inbounds so test to see if the ray cell is a wall block
                    if(map[nTestY * nMapWidth + nTestX]== '#'){
                        bHitWall = true;

                        vector<pair<float, float>> p; //distance, dot_product

                        for (int tx = 0; tx < 2; tx++)
                        for(int ty = 0; ty < 2; ty++){
                            float vy = (float)nTestY + ty - fPlayerY;
                            float vx = (float)nTestX + tx - fPlayerX;
                            float d = sqrt(vx*vx + vy*vy);
                            float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
                            p.push_back(make_pair(d, dot));
                        }

                        //Sort Pairs from closest to furthest
                        sort(p.begin(), p.end(), [](const pair<float, float> &left, const pair<float, float> &right) {return left.first < right.first;});

                        float fBound = 0.01;
                        if (acos(p.at(0).second) < fBound)
                            bBoundary = true;
                        if(acos(p.at(1).second) <  fBound)
                            bBoundary = true;
                        if(acos(p.at(2).second) <  fBound)
                            bBoundary = true;
                    }
                }
            }

            //Calculate distance to ceiling and floor
            int nCeiling = (float)(nScreenheight / 2.0) - nScreenheight/((float)FDistanceToWall);
            int nFloor = nScreenheight - nCeiling;

            //shader
            short nShade;

            for(int y = 0; y < nScreenheight; y++)
            {
                if(y < nCeiling)
                    screen[y*nScreenwidth + x] = ' ';
                //shader
                else if(y > nCeiling && y <= nFloor)
                {
                    if (FDistanceToWall <= fDepth/4.0f)         screen[y*nScreenwidth + x] = 0x2588;  //Very close
                    else if (FDistanceToWall < fDepth/3.0f)     screen[y*nScreenwidth + x] = 0x2593;
                    else if (FDistanceToWall < fDepth/2.0f)     screen[y*nScreenwidth + x] = 0x2592;
                    else if (FDistanceToWall < fDepth)          screen[y*nScreenwidth + x] = 0x2591;
                    else                                        screen[y*nScreenwidth + x] = ' ';     //Too far way
                    if(bBoundary) screen[y*nScreenwidth + x] = ' ';
                }
                else{
                    //Shade floor based on distance
                    float b = 1.0f - (((float)y - nScreenheight / 2.0f) /  ((float)nScreenheight / 2.0f));
                    if(b < 0.25)       nShade = '#';
                    else if (b < 0.5)  nShade = 'x';
                    else if (b < 0.75) nShade = '.';
                    else if (b < 0.9) nShade ='-';
                    else               nShade = ' ';

                    screen[y*nScreenwidth + x] = nShade;
                }
            }
        }

        //Display stats
        swprintf_s(screen, 60, L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f", fPlayerX,fPlayerY, fPlayerA, 1.0f / fElapsedTime);

        //Display Map
        for (int nx = 0; nx < nMapWidth; nx++)
            for(int ny = 0; ny < nMapWidth; ny++){
                screen[(ny + 1)* nScreenwidth + nx] = map[ny * nMapWidth + nx];
            }

        screen[((int)fPlayerY + 1) * nScreenwidth + (int)fPlayerX] = 'P';
        screen[nScreenwidth * nScreenheight - 1] = '\0';
        WriteConsoleOutputCharacterW(hConsole, screen, nScreenwidth * nScreenheight, {0, 0}, &dwBytesWritten);
   }

    return 0;
}
