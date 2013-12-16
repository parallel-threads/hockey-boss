<?php
/**
  @file
  @brief This file defines the API for creating and
    manipulating the Stat content types used by the
    Hockey Boss site.
*/

// ----- I. Content Types -----

/// The Player Game Stat type name.
define ('hockey_boss_stat_PGS_TYPE', 'hockey_boss_stat_pgs');

/// The Player Performance Stat type name.
define ('hockey_boss_stat_PPS_TYPE', 'hockey_boss_stat_pps');

/**
  @brief Returns an array listing the stat types
    defined by this module.
  @return (array) the stat types.
*/
function hockey_boss_stat_get_stat_types () {
  return array (
    hockey_boss_stat_PGS_TYPE => array (
      'name'          => 'Player Game Stat',
      'description'   => 'Represents player game statistics.',
      'create_fields' => array (
        'module'   => 'hockey_boss_stat',
        'callback' => 'hockey_boss_stat_create_pgs_fields'
    )),
    hockey_boss_stat_PPS_TYPE => array (
      'name'                   => 'Player Performance Stat',
      'description'            => 'Represents player performance statistics.'
  ));
}

/**
  @brief Creates the Player Game Stat fields.
  @param (array) the Player Game Stat type.
*/
function hockey_boss_stat_create_pgs_fields ($type) {
  module_load_include ('inc', 'stat', 'stat.field');
  stat_create_entity_reference ($type, $type->type . '_game', t ('Game'), 1);
}

/// Creates the stat types defined by this module.
function hockey_boss_stat_create_stat_types () {
  module_load_include ('inc', 'stat');
  stat_create_stat_types (hockey_boss_stat_get_stat_types ());
}

/// Deletes the stat types defined by this module.
function hockey_boss_stat_delete_stat_types () {
  module_load_include ('inc', 'stat');
  stat_delete_stat_types (hockey_boss_stat_get_stat_types ());
}
