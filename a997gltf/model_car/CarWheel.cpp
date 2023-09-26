#include "CarWheel.h"

int CarWheel::moveSubj(CarWheel* pGS) {
    float wheelRotationSpeedDg = 3;
    pGS->ownSpeed.setEulerDg( wheelRotationSpeedDg,0,0);
    applySpeeds(pGS);
    return 1;
}

