YAMS
====

Yet-another Arduino Menu System

 *  Created on: Jun 7, 2014  -- and still under construction
 *      Author: glenn burgess
 *
 * Yet Another Menu System
 *
 * Design principles:
 * . Support hierarchical menus
 * . Class based
 * . Default display routine
 * . Default menu actions for up/down/left/right/select
 * . Override default actions in subclasses for specific menus to encapsulate routines
 * . Common menu items types for value setting etc.
 *
 * Configurable: whether menu lists loop back or not
 *
 * Example usage:
 * Menu mm("Main Menu");
 *   //beneath is list of menu items needed to build the menu
 *   Menu runningtime    = Menu("Get/Set Runtime", false);
 *
 *     MenuValue runDays      = MenuValue("Days", 0);
 *     MenuValue runHours     = MenuValue("Hours", 0);
 *
 * mm.addChild(runingTime);
 * runningTime.addChild(runDays, runHours);
 *
