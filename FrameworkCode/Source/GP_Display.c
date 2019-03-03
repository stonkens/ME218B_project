/****************************************************************************
 Module
   GP_Display.c

 Revision
   1.0.1

 Description
   This is the dispaly Module for the Gameplay test harness.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 02/24/19       bibit   converted from Ed's MW display to Gameplay display
 02/07/12 00:36 jec     converted to Events & Services Framework Gen2
 02/21/05 16:20 jec     Began coding
****************************************************************************/


#include "ES_Configure.h"
#include "ES_Framework.h"
#include "MasterHSM.h"
#include "GamePlayHSM.h"
#include "CollisionAvoidanceHSM.h"
#include "CollectingSM.h"
#include "OrientingSM.h"
#include "RecyclingSM.h"
#include "LandfillingSM.h"
#include "KeyMapperService.h"
#include <stdio.h>
/*
void UpdateDisplay(void)
{
  MasterState_t               MasterState             = QueryMasterSM();
  GamePlayState_t             GamePlayState           = QueryGamePlaySM();
  CollectingState_t           CollectingState         = QueryCollectingSM();
  OrientingState_t            OrientingState          = QueryOrientingSM();
  RecyclingState_t            RecyclingState          = QueryRecyclingSM();
  LandfillingState_t          LandfillingState        = QueryLandfillingSM();
  CollisionAvoidanceState_t   CollisionAvoidanceState = QueryCollisionAvoidanceSM();

  // now update the display
  printf("\r");
  switch (MasterState)
  {
    case WaitingForStart:
    {
      printf(" WaitingForStart                 ");
    }
    break;
    case GamePlay:
    {
      printf(" GamePlay            ");
    }
    break;
    case GameEnded:
    {
      printf(" GameEnded ");
    }
    break;
    case CollisionAvoidance:
    {
      printf(" CollisionAvoidance  ");
    }
    break;
  }

  switch (GamePlayState)
  {
    case CollectingGarbage:
    {
      printf(" Collecting  ");
    }
    break;
    case Recycling:
    {
      printf(" Recycling ");
    }
    break;
    case Landfilling
    {
      printf(" Landfilling     ");
    }
    break;
  }

  switch (CollectingState)
  {
    case Orienting:
    {
      printf(" Orienting ");
    }
    break;
    case Driving2Target:
    {
      printf(" Driving2Target      ");
    }
    break;
    case Roaming:
    {
      printf(" Roaming   ");
    }
    break;
  }
  switch (OrientingState)
  {
    case Measuring:
    {
      printf(" Measuring       ");
    }
  }

  switch (RecyclingState)
  {
    case Orienting2Recycle:
    {
      printf(" Orienting2Recycle       ");
    }
    break;
    case Driving2Recycle:
    {
      printf(" Driving2Recycle     ");
    }
    break;
    case ApproachingRecycle:
    {
      printf(" ApproachingRecycle       ");
    }
    break;
    case T_SquaringWithWallCCW:
    {
      printf(" SquaringWithWallCCW      ");
    }
    break;
    case ReleasingTrash:
    {
      printf(" ReleasingTrash           ");
    }
    break;
  }

  switch (CollisionAvoidanceFrontState)
  {
    case Front_DrivingBackwards:
    {
      printf(" DrivingBackwards ");
    }
    break;
    case Front_TurningAway:
    {
      printf(" TurningAway      ");
    }
    break;
    case Front_GoingAroundRobot:
    {
      printf(" GoingAroundRobot ");
    }
    break;
  }
  switch (CollisionAvoidanceRearState)
  {
    case Rear_DrivingForward:
    {
      printf(" DrivingForward   ");
    }
    break;
    case Rear_TurningAway:
    {
      printf(" TurningAway      ");
    }
    break;
    case Rear_GoingAroundRobot:
    {
      printf(" GoingAroundRobot ");
    }
    break;
  }
  switch (TapeState)
  {
    case Looking4Tape:
    {
      printf(" Looking4Tape ");
    }
    break;
    case SawTape:
    {
      printf(" SawTape      ");
    }
    break;
  }
  
  printf(" %d ", HowManyRecyclingBalls());
  printf(" %d ", HowManyTrashBalls());
}


*/
