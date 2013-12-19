<?php
/**
  @file
  @brief This file defines the core content types
    and user roles used on the Hockey Boss site and
    provides an API for creating/modifying them.
*/

// ----- I. Content Types -----

/// The Coach Profile content type name.
define ('hockey_boss_COACH_PROFILE_TYPE', 'hockey_boss_coach_type');

/// The Game content type name.
define ('hockey_boss_GAME_TYPE', 'hockey_boss_game_type');

/// The Player Profile content type name.
define ('hockey_boss_PLAYER_PROFILE_TYPE', 'hockey_boss_player_type');

/// The Team Profile content type name.
define ('hockey_boss_TEAM_PROFILE_TYPE', 'hockey_boss_team_type');

/**
  @brief Defines a content type.
  @param $machine_name (string) the internal type name.
  @param $name (string) the human-readable type name.
  @param $description (string) a description of the type.
  @return (object) a content type.
*/
function hockey_boss_get_type ($machine_name, $name, $description) {
  $t = get_t (); 
  return node_type_set_defaults (
    array (
      'type'        => $machine_name,
      'name'        => $t ($name),
      'description' => $t ($description),
      'help'        => $t ($description),
      'has_title'   => true,
      'base'        => 'node_content',
      'custom'      => false,
      'modified'    => true,
      'locked'      => true,
      'module'      => 'stat'
  )); 
}

/**
  @brief Creates a content type.
  @param $machine_name (string) the internal type name.
  @param $name (string) the human-readable type name.
  @param $description (string) a description of the type.
  @return (object) a content type.
*/
function hockey_boss_create_type ($machine_name, $name, $description) {
  $type = hockey_boss_get_type ($machine_name, $name, $description);
  node_type_save ($type);
  return $type;
}

/// Creates the Coach Profile type.
function hockey_boss_create_coach_profile_type () {
  $type = hockey_boss_create_type (hockey_boss_COACH_PROFILE_TYPE, 'Coach Profile', 'Represents coach profiles.');

  // does the simple_field_create_entity_reference function need to have an argument allowing user entity references?
  // simple_field_create_entity_reference ($type, $type->type . '_user', t ('Coach User Account'), array ('user'));

  // simple_field_create_entity_reference ($type, $type->type . '_team', t ('Team Profile'));
}

/// Creates the Team Profile type.
function hockey_boss_create_team_profile_type () {
  $type = hockey_boss_create_type (hockey_boss_TEAM_PROFILE_TYPE, 'Team Profile', 'Represents amateur hockey teams.');
}
