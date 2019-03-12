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
//#include "CollectingSM.h" Triangulation method
#include "CollectingV2SM.h"
#include "OrientingSM.h"
#include "RecyclingSM.h"
#include "LandfillingSM.h"
#include "KeyMapperService.h"
#include <stdio.h>
#include "ColorService.h"
#include "BallDumpingSM.h"
#include "BallProcessingSM.h"

bool BotDirection;

void UpdateDisplay(void)
{
  MasterState_t               MasterState             = QueryMasterSM();
  GamePlayState_t             GamePlayState           = QueryGamePlaySM();
  CollectingState_t           CollectingState         = QueryCollectingV2SM();
  //OrientingState_t            OrientingState          = QueryOrientingSM();
  RecyclingState_t            RecyclingState          = QueryRecyclingSM();
  LandfillingState_t          LandfillingState        = QueryLandfillingSM();
  CollisionAvoidanceState_t   CollisionAvoidanceState = QueryCollisionAvoidanceHSM();
  /*
  // now update the display
  printf("\r");
  switch (MasterState)
  {
    case WaitingForStart:
    {
      printf("  WaitingForStart   ");
    }
    break;
    case GamePlay:
    {
      printf("     GamePlay       ");
    }
    break;
    case GameEnded:
    {
      printf("      GameEnded     ");
    }
    break;
    case CollisionAvoidance:
    {
      printf(" CollisionAvoidance ");
    }
    break;
  }

  switch (GamePlayState)
  {
    case CollectingGarbage:
    {
      printf("  Collecting ");
    }
    break;
    case Recycling:
    {
      printf("  Recycling  ");
    }
    break;
    case Landfilling:
    {
      printf(" Landfilling ");
    }
    break;
  }

  switch (CollectingState)
  {
    case Align2Landfill:
    {
      printf("  Align2Landfill  ");
    }
    break;
    case StraightDrive:
    {
      printf("   StraightDrive  ");
    }
    break;
    case TurnDrive:
    {
      printf("    TurnDrive     ");
    }
    break;
    case Prepare4Harvesting:
    {
      printf("Prepare4Harvesting");
    }
    break;
    case FindingTape:
    {
      printf("    FindingTape   ");
    }
    break;
  }
  

  switch (RecyclingState)
  {
    case Driving2LandfillR:
    {
      printf(" Driving2LanfillIR  ");
    }
    break;
    case MovingBodyRotation:
    {
      printf(" MovingBodyRotation ");
    }
    break;
    case Orienting2LandfillR:
    {
      printf(" Orienting2LanfillIR");
    }
    break;
    case Orienting2Recycle:
    {
      printf(" Orienting2Recycle  ");
    }
    break;
    case Driving2Recycle:
    {
      printf("  Driving2Recycle   ");
    }
    break;
    case ApproachingRecycle:
    {
      printf(" ApproachingRecycle ");
    }
    break;
    case Preparing4Recycle:
    {
      printf(" Preparing4Recycle  ");
    }
    break;
    case DumpingRecycle:
    {
      printf("   DumpingRecycle   ");
    }
    break;
    case RecoveringFromRecycle:
    {
      printf("RecoveringFromRecy. ");
    }
    break;
  }
  switch (LandfillingState)
  {
    case Orienting2RecycleL:
    {
      printf(" Orienting2RecycleL  ");
    }
    break;
    case Driving2RecycleL:
    {
      printf("   Driving2RecycleL   ");
    }
    break;
        case ReversingFromRecycleL:
    {
      printf(" ReversingFromRecycle ");
    }
    break;
    case Orienting2Landfill:
    {
      printf(" Orienting2Landfill  ");
    }
    break;
    case Driving2Landfill:
    {
      printf("  Driving2Landfill   ");
    }
    break;
    case ApproachingLandfill:
    {
      printf(" ApproachingLandfill ");
    }
    break;
    case Preparing4Landfill:
    {
      printf(" Preparing4Landfill  ");
    }
    break;
    
    case DumpingLandfill:
    {
      printf("   DumpingLandfill   ");
    }
    break;
    
    case RecoveringFromDump:
    {
      printf(" RecoveringFromDump ");
    }
    break;
  }
  switch (CollisionAvoidanceState)
  {
    case MovingBackwards:
    {
      printf(" MovingBackwards ");
    }
    break;
    case QuarterTurn:
    {
      printf("   QuarterTurn   ");
    }
    break;
    case MovingForward:
    {
      printf("  MovingForward  ");
    }
    break;
  }  
  BotDirection = QueryBotDirection();
  printf(" %d ", BotDirection);
  printf(" %d ", QueryRecycleBalls());
  printf(" %d ", QueryLandFillBalls());
  printf(" %d ", QueryTeam());
*/

}



