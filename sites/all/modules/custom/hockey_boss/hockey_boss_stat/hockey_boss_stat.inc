<?php
/**
  @file
  @brief This file defines the API for creating and
    manipulating the Stat content types used by the
    Hockey Boss site.
*/

// ----- I. Content Types -----

function hockey_boss_stat_pgs_type () {
  module_load_include ('inc', 'hockey_boss');
  return array (
    'name'              => 'Player Game Stat',
    'description'       => 'Represents player game statistics.',
    'node_type'         => 'hockey_boss_stat_pgs',
    'subject_node_type' => hockey_boss_PLAYER_PROFILE_TYPE,
  );
}

function hockey_boss_stat_pps_type () {
  module_load_include ('inc', 'hockey_boss');
  return array (
    'name'              => 'Player Performance Stat',
    'description'       => 'Represents player performance statistics.',
    'node_type'         => 'hockey_boss_stat_pps',
    'subject_node_type' => hockey_boss_PLAYER_PROFILE_TYPE
  );
}

/**
  @brief Returns an array listing the stat types
    defined by this module.
  @return (array) the stat types.
*/
function hockey_boss_stat_get_stat_types () {
  module_load_include ('inc', 'hockey_boss');

  return array (
    hockey_boss_stat_pgs_type (),
    hockey_boss_stat_pps_type ()
  );
}

/**
  @brief Implements hook_stat_create_<node type>_stat_type_node_type ().
  @details
    Adds fields to the Player Game Stats node type.
*/
function hockey_boss_stat_stat_create_hockey_boss_stat_pgs_stat_type_node_type ($stat_node) {
  module_load_include ('inc', 'hockey_boss');
  simple_field_create_entity_reference ($stat_node, $stat_node->type . '_game', t ('Game'), hockey_boss_GAME_TYPE, 1);
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
