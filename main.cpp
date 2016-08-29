#include <iostream>
#include <stdio.h>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Audio.hpp>

//#include <Keyboard.hpp>


#include <math.h>
#include <ctime>

#define WW 800
#define WH 600
#define SCALE 4

#define START_ALTITUDE 10000
#define START_ANGLE 5

#define SL_TEMPERATURE  288.66
#define SL_PRESSURE  101325
#define SL_DENSITY 1.225

//Atmospheric Conditions
#define NUM_C 4
#define C_ALTITUDE 0
#define C_TEMPERATURE 1
#define C_PRESSURE 2
#define C_DENSITY 3

//Constants
#define GAS_CONSTANT 287
#define GRAVITY 9.81
#define GAMMA_AIR 1.4
#define G_LIMIT 2
#define SPEEDSCALE 1000

//Aircraft/Airfoil Information
#define MAX_THRUST 75000

#define NUM_AI 11
#define AI_SPAN 0
#define AI_AR 1
#define AI_AREA 2
#define AI_EFFICIENCY 3
#define AI_MAX_THRUST 4
#define AI_MASS 5

#define AI_A0 6
#define AI_A_FINITE 7
#define AI_ZERO_LIFT_ANGLE 8
#define AI_STALL_ANGLE 9
#define AI_ZERO_LIFT_DRAG 10

//Aerodynamics
#define NUM_AD 9
#define AD_ATTACK_ANGLE 0
#define AD_LIFT_COEFFICIENT 1
#define AD_DRAG_COEFFICIENT 2
#define AD_LIFT 3
#define AD_DRAG 4
#define AD_THRUST_AVAILABLE 5
#define AD_ACTUAL_THRUST 6
#define AD_MACH 7
#define AD_STALL 8

//Dynamics
#define NUM_D 7
#define D_V_X 1
#define D_V_Y 2
#define D_V_R 3
#define D_V_ANGLE 4
#define D_A_X 5
#define D_A_Y 6

//User Input
#define NUM_I 3
#define I_ANGLE 0
#define I_THROTTLE 1
#define I_FRAMERATE 2
#define I_OPEN_SWITCH 3
#define I_FLIP_SWITCH 4

#define NUM_S 13
#define S_CLOUD1 0
#define S_CLOUD2 1
#define S_CLOUD3 2
#define S_PLANE 3
#define S_PITCH_DIAL_SCALE 4
#define S_STALL_LIGHT 5
#define S_DASHBOARD 6

#define S_MACH_ARROW 7
#define S_VELOCITY_ARROW 8
#define S_ALTITUDE_ARROW 9

#define S_THROTTLE_BAR 10
#define S_A_Y_BAR 11

#define S_ATTACK_ANGLE_BAR 12



#define TRANSONIC_LB 0.7
#define TRANSONIC_UB 1.2

#define NUM_SOUNDS 2



using namespace std;
void getInputs(sf::Event*, float*);
void getConditions(float*);
void getAerodynamics(float*, float*, float*, float*);
void getDynamics(float*, float*, float*, float*, float*);
void updatePhysics(float*, float*, float*, float*, float*);
void displayEverything(sf::RenderWindow*, float*, float*, float*, float*, float*, sf::Sprite*);

int main(){
    //Setting up the window
    sf::RenderWindow window(sf::VideoMode(WW, WH), "Flight Simulator 2018");
    //window.setPosition(sf::Vector2i(200, 0));


    sf::Music song;
    if(!song.openFromFile("80s Action Tune.wav")){
        return -1;
    }
    song.play();
    song.setLoop(true);

    //Muh-Muh-Mach oonnne
    sf::SoundBuffer sbuffer[NUM_SOUNDS];
    sf::Sound mach[NUM_SOUNDS];

    char buffer[32];

    for(int x = 0; x < NUM_SOUNDS; x++){
        sprintf(buffer, "Mach%d.wav", x+1);
        if(!sbuffer[x].loadFromFile(buffer)){
            return -1;
        }
        mach[x].setBuffer(sbuffer[x]);
        mach[x].setVolume(10);
    }




    sf::Sprite sprites[NUM_S];

    //Airplane Sprite
    sf::Image spriteSheetImg;
    if(!spriteSheetImg.loadFromFile("SpriteSheet.png")){
    }
    spriteSheetImg.createMaskFromColor(sf::Color::Magenta);
    sf::Texture spriteSheetTxt;
    if(!spriteSheetTxt.loadFromImage(spriteSheetImg)){
    }
    for(int x = 0; x < NUM_S; x++){
        sprites[x].setTexture(spriteSheetTxt);
    }
    for(int x = 0; x < NUM_S; x++){
        if(x != S_PLANE && x != S_CLOUD1 && x != S_CLOUD2 && x != S_CLOUD3){
            sprites[x].setScale(SCALE, SCALE);
        }
        else if(x == S_PLANE){
            sprites[x].setScale(1, 1);
        }
        else if(x == S_CLOUD1 || x == S_CLOUD2 || x == S_CLOUD3){
            sprites[x].setScale(SCALE*(x + 1 - S_CLOUD1), SCALE*(x + 1 - S_CLOUD1));
        }
    }

    sprites[S_DASHBOARD].setTextureRect(sf::IntRect(5, 90, 89, 35));
    sprites[S_DASHBOARD].setPosition((WW - sprites[S_DASHBOARD].getGlobalBounds().width)/2, WH-(25+10)*SCALE);

    sprites[S_CLOUD1].setTextureRect(sf::IntRect(5, 25, 39, 21));
    sprites[S_CLOUD1].setPosition(WW/2, WH/4);
    sprites[S_CLOUD2].setTextureRect(sf::IntRect(5, 25, 39, 21));
    sprites[S_CLOUD2].setPosition(WW, WH/2);
    sprites[S_CLOUD3].setTextureRect(sf::IntRect(5, 25, 39, 21));
    sprites[S_CLOUD3].setPosition(WW*3/2, WH*3/4);

    sprites[S_PLANE].setTextureRect(sf::IntRect(5, 5, 61, 15));
    sprites[S_PLANE].setOrigin(30.5, 7.5);

    sprites[S_PITCH_DIAL_SCALE].setTextureRect(sf::IntRect(165, 5, 25, 25));

    sprites[S_STALL_LIGHT].setTextureRect(sf::IntRect(85, 50, 5, 5));
    sprites[S_STALL_LIGHT].setPosition(sprites[S_DASHBOARD].getGlobalBounds().left+55*SCALE, sprites[S_DASHBOARD].getGlobalBounds().top + 7*SCALE);

    sprites[S_MACH_ARROW].setTextureRect(sf::IntRect(145, 10, 3, 10));
    sprites[S_MACH_ARROW].setOrigin(1.5, 1.5);
    sprites[S_MACH_ARROW].setPosition(sprites[S_DASHBOARD].getGlobalBounds().left + 69.5*SCALE, sprites[S_DASHBOARD].getGlobalBounds().top + 22.5*SCALE);

    sprites[S_VELOCITY_ARROW].setTextureRect(sf::IntRect(140, 10, 3, 10));
    sprites[S_VELOCITY_ARROW].setOrigin(1.5, 1.5);
    sprites[S_VELOCITY_ARROW].setPosition(sprites[S_DASHBOARD].getGlobalBounds().left + 69.5*SCALE, sprites[S_DASHBOARD].getGlobalBounds().top + 22.5*SCALE);
    sprites[S_ALTITUDE_ARROW].setTextureRect(sf::IntRect(140, 10, 3, 10));
    sprites[S_ALTITUDE_ARROW].setOrigin(1.5, 1.5);
    sprites[S_ALTITUDE_ARROW].setPosition(sprites[S_DASHBOARD].getGlobalBounds().left + 19.5*SCALE, sprites[S_DASHBOARD].getGlobalBounds().top + 22.5*SCALE);

    sprites[S_THROTTLE_BAR].setTextureRect(sf::IntRect(155, 5, 5, 1));
    sprites[S_THROTTLE_BAR].setOrigin(0, 0.5);

    sprites[S_A_Y_BAR].setTextureRect(sf::IntRect(155, 5, 5, 1));
    sprites[S_A_Y_BAR].setOrigin(0, 0.5);

    sprites[S_ATTACK_ANGLE_BAR].setTextureRect(sf::IntRect(150, 10, 11, 2));






    //Set up user input array
    float userInputs[NUM_I];
    userInputs[I_ANGLE] = START_ANGLE;
    userInputs[I_FRAMERATE] = 10;

    //Set up atmospheric conditions array
    float conditions[4];
    conditions[C_ALTITUDE] = START_ALTITUDE;
    getConditions(conditions);

    //Set up aircraft information array
    float aircraftInfo[NUM_AI];
    aircraftInfo[AI_SPAN] = 10;
    aircraftInfo[AI_AR] = 8;
    aircraftInfo[AI_AREA] = pow(aircraftInfo[AI_SPAN], 2)/aircraftInfo[AI_AR];
    aircraftInfo[AI_EFFICIENCY] = 0.9;
    aircraftInfo[AI_MAX_THRUST] = MAX_THRUST;

    aircraftInfo[AI_MASS] = 3000;
    aircraftInfo[AI_ZERO_LIFT_DRAG] = 0.05;
    aircraftInfo[AI_A0] = 0.1;
    aircraftInfo[AI_A_FINITE] = aircraftInfo[AI_A0]/(1+57.3*aircraftInfo[AI_A0]/(M_PI*aircraftInfo[AI_EFFICIENCY]*aircraftInfo[AI_AR]));
    aircraftInfo[AI_ZERO_LIFT_ANGLE] = 0;
    aircraftInfo[AI_STALL_ANGLE] = 15;



    //Set up aerodynamics array
    float aerodynamics[NUM_AD];
    aerodynamics[AD_ATTACK_ANGLE] = userInputs[I_ANGLE];
    aerodynamics[AD_LIFT_COEFFICIENT] = aircraftInfo[AI_A_FINITE]*(aerodynamics[AD_ATTACK_ANGLE]-aircraftInfo[AI_ZERO_LIFT_ANGLE]);
    aerodynamics[AD_DRAG_COEFFICIENT] = aircraftInfo[AI_ZERO_LIFT_DRAG] + pow(aerodynamics[AD_LIFT_COEFFICIENT], 2)/(M_PI*aircraftInfo[AI_EFFICIENCY]*aircraftInfo[AI_AR]);

    float dynamics[NUM_D];
    dynamics[D_V_X] = sqrt(2*aircraftInfo[AI_MASS]*GRAVITY/(conditions[C_DENSITY]*aircraftInfo[AI_AREA]*aerodynamics[AD_LIFT_COEFFICIENT])); //Start at L = W for Steady flight
    dynamics[D_V_Y] = 0;
    dynamics[D_V_R] = sqrt(pow(dynamics[D_V_X], 2) + pow(dynamics[D_V_Y], 2));
    dynamics[D_V_ANGLE] = atan2(dynamics[D_V_Y], dynamics[D_V_X])*180/M_PI;
    dynamics[D_A_X] = 0;
    dynamics[D_A_Y] = 0;

    aerodynamics[AD_THRUST_AVAILABLE] = aircraftInfo[AI_MAX_THRUST]*conditions[C_DENSITY]/SL_DENSITY;

    userInputs[I_THROTTLE] = 0.5*conditions[C_DENSITY]*pow(dynamics[D_V_X], 2)*aircraftInfo[AI_AREA]*aerodynamics[AD_DRAG_COEFFICIENT]/aerodynamics[AD_THRUST_AVAILABLE]; //Start at T = D for Steady flight
    aerodynamics[AD_ACTUAL_THRUST] = userInputs[I_THROTTLE]*aerodynamics[AD_THRUST_AVAILABLE];
    float currentMach = 0;

    while(window.isOpen()){
        //window.setFramerateLimit(userInputs[I_FRAMERATE]);

        sf::Event event;
        while(window.pollEvent(event)){
            if(event.type == sf::Event::Closed || event.key.code == sf::Keyboard::Escape){
                window.close();
            }
            if(event.type == sf::Event::KeyPressed){
                getInputs(&event, userInputs);
            }
        }


        getConditions(conditions);
        getAerodynamics(aerodynamics, aircraftInfo, dynamics, conditions);
        getDynamics(dynamics, aircraftInfo, aerodynamics, userInputs, conditions);
        displayEverything(&window, conditions, aircraftInfo, aerodynamics, dynamics, userInputs, sprites);
        updatePhysics(conditions, dynamics, aerodynamics, aircraftInfo, userInputs);


        if(currentMach < 1 && aerodynamics[AD_MACH] > 1){
            mach[0].play();
        }
        if(currentMach < 2 && aerodynamics[AD_MACH] > 2){
            mach[1].play();
        }


        currentMach = aerodynamics[AD_MACH];

    }
}

void getInputs(sf::Event* event, float userInputs[]){
    bool pitchUp = (*event).key.code == sf::Keyboard::Numpad5;
    bool pitchDown = (*event).key.code == sf::Keyboard::Numpad8;
    bool throttleUp = (*event).key.code == sf::Keyboard::Add;
    bool throttleDown = (*event).key.code == sf::Keyboard::Subtract;

    bool frameRateUp = (*event).key.code == sf::Keyboard::PageUp;
    bool frameRateDown = (*event).key.code == sf::Keyboard::PageDown;

    bool openswitch = (*event).key.code == sf::Keyboard::Num0;

    userInputs[I_ANGLE] += ((int)pitchUp - (int)pitchDown);
    if(userInputs[I_ANGLE] > 180){
        userInputs[I_ANGLE] -= 360;
    }
    else if(userInputs[I_ANGLE] <= -180){
        userInputs[I_ANGLE] += 360;
    }

    userInputs[I_THROTTLE] += ((int)throttleUp - (int)throttleDown)*0.05;
    userInputs[I_FRAMERATE] += ((int)frameRateUp - (int)frameRateDown)*10;

    if(openswitch == true){
            userInputs[I_OPEN_SWITCH] = not(userInputs[I_OPEN_SWITCH]);
    }


    if(userInputs[I_THROTTLE] > 1){
        userInputs[I_THROTTLE] = 1;
    }
    if(userInputs[I_THROTTLE] < 0){
        userInputs[I_THROTTLE] = 0;
    }

    if(userInputs[I_FRAMERATE] < 0){
        userInputs[I_FRAMERATE] = 1;
    }
}

void getConditions(float conditions[]){ //Based only on altitude

    bool gradient_region;
    float alt_a;
    float base_T;
    float base_P;
    float base_alt;

    if((conditions[C_ALTITUDE]) >= 0 && conditions[C_ALTITUDE] < 11000){
        gradient_region = true;
        alt_a = -6.5; // K/km
        base_T = 288.16; // K
        base_P = 101325; //Pa
        base_alt = 0; // m
    }
    if(conditions[C_ALTITUDE] >= 11000 && conditions[C_ALTITUDE] < 25000){
        gradient_region = false;
        base_T = 216.66;
        base_P = 22700;
        base_alt = 11000;
    }
    if(conditions[C_ALTITUDE] >= 25000 && conditions[C_ALTITUDE] < 47000){
        gradient_region = true;
        alt_a = 3;
        base_T = 216.66;
        base_P = 2527.3;
        base_alt = 25000;
    }
    if(conditions[C_ALTITUDE] >= 47000 && conditions[C_ALTITUDE] < 53000){
        gradient_region = false;
        base_T = 282.66;
        base_P = 125.58;
        base_alt = 47000;
    }
    if(conditions[C_ALTITUDE] >= 53000){// && alt < 79000){
        gradient_region = true;
        alt_a = -4.5;
        base_T = 282.66;
        base_P = 61.493;
        base_alt = 53000;
    }
    if(gradient_region == true){
        conditions[C_TEMPERATURE] = base_T + alt_a*(conditions[C_ALTITUDE] - base_alt)/1000;
        conditions[C_PRESSURE] = base_P*pow(conditions[C_TEMPERATURE]/base_T, -GRAVITY*1000/(alt_a*GAS_CONSTANT));
    }
    if(gradient_region == false){
        conditions[C_TEMPERATURE] = base_T;
        conditions[C_PRESSURE] = base_P*exp(-GRAVITY*(conditions[C_ALTITUDE] - base_alt)/(GAS_CONSTANT*conditions[C_TEMPERATURE]));
    }
    conditions[C_DENSITY] = conditions[C_PRESSURE]/(GAS_CONSTANT*conditions[C_TEMPERATURE]);
    //v_sound = sqrt(GAMMA_AIR*GAS_CONSTANT*temperature);
}

void getAerodynamics(float aerodynamics[], float aircraftInfo[], float dynamics[], float conditions[]){

    aerodynamics[AD_STALL] = (float)false;
    static float previousLift = 0;

    aerodynamics[AD_MACH] = dynamics[D_V_R]/sqrt(GAMMA_AIR*GAS_CONSTANT*conditions[C_TEMPERATURE]);
    if(aerodynamics[AD_ATTACK_ANGLE] > aircraftInfo[AI_STALL_ANGLE] || aerodynamics[AD_ATTACK_ANGLE] < -aircraftInfo[AI_STALL_ANGLE]){
        aerodynamics[AD_STALL] = (float)true;
        aerodynamics[AD_LIFT_COEFFICIENT] = 0;
    }

    //Low Speed
    if((bool)aerodynamics[AD_STALL] == false){
        aerodynamics[AD_LIFT_COEFFICIENT] = aircraftInfo[AI_A_FINITE]*(aerodynamics[AD_ATTACK_ANGLE] - aircraftInfo[AI_ZERO_LIFT_ANGLE]);
        if(aerodynamics[AD_MACH] < 1){//Subsonic
            aerodynamics[AD_LIFT_COEFFICIENT] = aerodynamics[AD_LIFT_COEFFICIENT]/sqrt(1-pow(aerodynamics[AD_MACH], 2));
            aerodynamics[AD_DRAG_COEFFICIENT] = aircraftInfo[AI_ZERO_LIFT_DRAG] + pow(aerodynamics[AD_LIFT_COEFFICIENT], 2)/(M_PI*aircraftInfo[AI_EFFICIENCY]*aircraftInfo[AI_AR]);
        }//Transonic

        else if(aerodynamics[AD_MACH] >= TRANSONIC_LB && aerodynamics[AD_MACH] < TRANSONIC_UB){
            aerodynamics[AD_LIFT_COEFFICIENT] = aerodynamics[AD_LIFT_COEFFICIENT]/sqrt(1-pow(TRANSONIC_LB, 2));
            aerodynamics[AD_DRAG_COEFFICIENT] = aircraftInfo[AI_ZERO_LIFT_DRAG] + pow(aerodynamics[AD_LIFT_COEFFICIENT], 2)/(M_PI*aircraftInfo[AI_EFFICIENCY]*aircraftInfo[AI_AR]);
        }
        else{//Supersonic
            aerodynamics[AD_LIFT_COEFFICIENT] = 4*(aerodynamics[AD_ATTACK_ANGLE]*M_PI/180)/sqrt(pow(aerodynamics[AD_MACH], 2) - 1);
            aerodynamics[AD_DRAG_COEFFICIENT] = aircraftInfo[AI_ZERO_LIFT_DRAG] + 4*pow(aerodynamics[AD_ATTACK_ANGLE]*M_PI/180, 2)/sqrt(pow(aerodynamics[AD_MACH], 2) - 1);
        }

    }


    aerodynamics[AD_LIFT] = (0.5*conditions[C_DENSITY]*pow(dynamics[D_V_R], 2)*aircraftInfo[AI_AREA]*aerodynamics[AD_LIFT_COEFFICIENT]+previousLift)/2;
    aerodynamics[AD_DRAG] = 0.5*conditions[C_DENSITY]*pow(dynamics[D_V_R], 2)*aircraftInfo[AI_AREA]*aerodynamics[AD_DRAG_COEFFICIENT];

    previousLift = aerodynamics[AD_LIFT];

}

void getDynamics(float dynamics[], float aircraftInfo[], float aerodynamics[], float userInputs[], float conditions[]){
    dynamics[D_A_X] = (aerodynamics[AD_ACTUAL_THRUST]*cos(userInputs[I_ANGLE]*M_PI/180) - aerodynamics[AD_DRAG]*cos(dynamics[D_V_ANGLE]*M_PI/180) - aerodynamics[AD_LIFT]*sin(dynamics[D_V_ANGLE]*M_PI/180))/aircraftInfo[AI_MASS];
    dynamics[D_A_Y] = (aerodynamics[AD_ACTUAL_THRUST]*sin(userInputs[I_ANGLE]*M_PI/180) + aerodynamics[AD_LIFT]*cos(dynamics[D_V_ANGLE]*M_PI/180) - aerodynamics[AD_DRAG]*sin(dynamics[D_V_ANGLE]*M_PI/180))/aircraftInfo[AI_MASS] - GRAVITY;
}

void updatePhysics(float conditions[], float dynamics[], float aerodynamics[], float aircraftInfo[], float userInputs[]){
    conditions[C_ALTITUDE] += dynamics[D_V_Y]/SPEEDSCALE;
    if(conditions[C_ALTITUDE] < 0){
        conditions[C_ALTITUDE] = 0;
    }
    dynamics[D_V_X] += dynamics [D_A_X]/SPEEDSCALE;
    dynamics[D_V_Y] += dynamics[D_A_Y]/SPEEDSCALE;
    dynamics[D_V_R] = sqrt(pow(dynamics[D_V_X], 2) + pow(dynamics[D_V_Y], 2));
    dynamics[D_V_ANGLE] = atan2(dynamics[D_V_Y], dynamics[D_V_X])*180/M_PI;
    aerodynamics[AD_ATTACK_ANGLE] = userInputs[I_ANGLE] - dynamics[D_V_ANGLE];
    aerodynamics[AD_THRUST_AVAILABLE] = aircraftInfo[AI_MAX_THRUST]*conditions[C_DENSITY]/SL_DENSITY;
    aerodynamics[AD_ACTUAL_THRUST] = userInputs[I_THROTTLE]*aerodynamics[AD_THRUST_AVAILABLE];
}
void displayEverything(sf::RenderWindow* window, float conditions[], float aircraftInfo[], float aerodynamics[], float dynamics[], float userInputs[], sf::Sprite sprites[]){

    float angle;

    if(userInputs[I_ANGLE] <= 90 && userInputs[I_ANGLE] >= -90){
        sprites[S_PITCH_DIAL_SCALE].setScale(SCALE, SCALE);
        sprites[S_PITCH_DIAL_SCALE].setTextureRect(sf::IntRect(165, 56 - 3*((int)userInputs[I_ANGLE]/5), 25, 29));
        sprites[S_PITCH_DIAL_SCALE].setPosition((WW-sprites[S_PITCH_DIAL_SCALE].getGlobalBounds().width)/2, sprites[S_DASHBOARD].getGlobalBounds().top + 8*SCALE + 3*((int)userInputs[I_ANGLE]%5)*SCALE/5);
    }
    else if(userInputs[I_ANGLE] >= 90){
        sprites[S_PITCH_DIAL_SCALE].setScale(SCALE, -SCALE);
        sprites[S_PITCH_DIAL_SCALE].setTextureRect(sf::IntRect(165, 56 - 3*(((int)(179-userInputs[I_ANGLE]))/5), 25, 29));
        sprites[S_PITCH_DIAL_SCALE].setPosition((WW-sprites[S_PITCH_DIAL_SCALE].getGlobalBounds().width)/2, sprites[S_DASHBOARD].getGlobalBounds().top + 34*SCALE + 3*((int)userInputs[I_ANGLE]%5)*SCALE/5);
    }
    else if(userInputs[I_ANGLE] < -90){
    }

    if((bool)aerodynamics[AD_STALL] == true){
        sprites[S_STALL_LIGHT].setTextureRect(sf::IntRect(85, 55, 5, 5));
    }
    else{
        sprites[S_STALL_LIGHT].setTextureRect(sf::IntRect(85, 50, 5, 5));
    }

    sprites[S_MACH_ARROW].setRotation(45 + aerodynamics[AD_MACH]*45);
    sprites[S_VELOCITY_ARROW].setRotation(45 + dynamics[D_V_R]*45/100);
    sprites[S_ALTITUDE_ARROW].setRotation(45 + conditions[C_ALTITUDE]*45/10000);

    sprites[S_THROTTLE_BAR].setPosition(sprites[S_DASHBOARD].getGlobalBounds().left + 83*SCALE, sprites[S_DASHBOARD].getGlobalBounds().top + 33.5*SCALE - userInputs[I_THROTTLE]*22*SCALE);

    sprites[S_A_Y_BAR].setPosition(sprites[S_DASHBOARD].getGlobalBounds().left + SCALE, sprites[S_DASHBOARD].getGlobalBounds().top + 22.5*SCALE - dynamics[D_A_Y]*3*SCALE/GRAVITY);
    sprites[S_ATTACK_ANGLE_BAR].setPosition((WW-sprites[S_ATTACK_ANGLE_BAR].getGlobalBounds().width)/2, sprites[S_DASHBOARD].getGlobalBounds().top + 22*SCALE + (userInputs[I_ANGLE] - aerodynamics[AD_ATTACK_ANGLE])*3*SCALE/5);

    sprites[S_PLANE].setPosition(dynamics[D_V_X], WH/2 - dynamics[D_V_Y]);
    sprites[S_PLANE].setRotation(-userInputs[I_ANGLE]);

    for(int x = S_CLOUD1; x <= S_CLOUD3; x++){
        if(sprites[x].getPosition().x + sprites[x].getGlobalBounds().width < 0 && dynamics[D_V_X] > 0){
            sprites[x].setPosition(WW/2 + 500*cos(dynamics[D_V_ANGLE]*M_PI/180), WH*(1+x)/4 - 500*sin(dynamics[D_V_ANGLE]*M_PI/180));
        }
        sprites[x].move(-dynamics[D_V_X]/SPEEDSCALE/(1 + S_CLOUD3 - x), dynamics[D_V_Y]/SPEEDSCALE/(1 + S_CLOUD3 - x));
    }


    sf::Color color(0, 255*(1-conditions[C_ALTITUDE]/60000), 255);
    (*window).clear(color);

    (*window).draw(sprites[S_CLOUD1]);
    (*window).draw(sprites[S_CLOUD2]);
    (*window).draw(sprites[S_PLANE]);
    (*window).draw(sprites[S_CLOUD3]);

    for(int x = S_PLANE + 1; x < NUM_S; x++){
        (*window).draw(sprites[x]);
    }

    //(*window).draw(sprites[S_SWITCH]);
    //(*window).draw(sprites[S_FLIP_SWITCH_CLOSED]);
    //(*window).draw(sprites[S_FLIP_SWITCH_OPEN]);

    (*window).display();

/*
    cout << "Altitude: " << conditions[C_ALTITUDE]/1000 << " km, Temperature: " << conditions[C_TEMPERATURE] << " K, Pressure: " << conditions[C_PRESSURE]/1000 << " kPa, Density: " << conditions[C_DENSITY] << " kg/m^3" << endl;
    cout << "Input Angle: " << userInputs[I_ANGLE] << " deg, Input Throttle: " << userInputs[I_THROTTLE]*100 << " %" << endl;
    cout << "Mach: " << aerodynamics[AD_MACH] << ", Attack Angle: " << aerodynamics[AD_ATTACK_ANGLE] << " deg, Lift Coefficient: " << aerodynamics[AD_LIFT_COEFFICIENT] << ", Drag Coefficient: " << aerodynamics[AD_DRAG_COEFFICIENT] << ", Lift: " << aerodynamics[AD_LIFT] << " N, Drag: " << aerodynamics[AD_DRAG] << " N, Thrust: " << aerodynamics[AD_ACTUAL_THRUST] << " N" << endl;
    cout << "Vx: " << dynamics[D_V_X] << " m/s, Vy: " << dynamics[D_V_Y] << " m/s, Ax: " << dynamics[D_A_X] << " m/s^2, Ay: " << dynamics[D_A_Y] << endl;

    cout << endl;
*/


    //cout << dynamics[D_V_ANGLE] << " " << sprites[S_CLOUD1].getPosition().x << " " << sprites[S_CLOUD1].getPosition().y << endl;
}

