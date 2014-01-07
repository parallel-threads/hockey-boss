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

  node_add_body_field ($type, 'Bio');
  simple_field_create_entity_reference ($type, $type->type . '_user', t ('User Account'), array (), 'user');
  simple_field_create_entity_reference ($type, $type->type . '_team', t ('Team'), array (hockey_boss_TEAM_PROFILE_TYPE));
}

/// Creates the Game type.
function hockey_boss_create_game_type () {
  $type = hockey_boss_create_type (hockey_boss_GAME_TYPE, 'Game', 'Represents matches.');

  node_add_body_field ($type);
  simple_field_create_textfield ($type, $type->type . '_scores', t ('Scores'));
  simple_field_create_entity_reference ($type, $type->type . '_teams', t ('Teams'), array (hockey_boss_TEAM_PROFILE_TYPE), 'node', 2);
}

/// Creates the Player Profile type.
function hockey_boss_create_player_profile_type () {
  $type = hockey_boss_create_type (hockey_boss_PLAYER_PROFILE_TYPE, 'Player Profile', 'Represents amateur hockey players.');

  node_add_body_field ($type, 'Bio');
  simple_field_create_entity_reference ($type, $type->type . '_user', t ('User Account'), array (), 'user');
  simple_field_create_entity_reference ($type, $type->type . '_team', t ('Team'), array (hockey_boss_TEAM_PROFILE_TYPE));
}

/// Creates the Team Profile type.
function hockey_boss_create_team_profile_type () {
  $type = hockey_boss_create_type (hockey_boss_TEAM_PROFILE_TYPE, 'Team Profile', 'Represents amateur hockey teams.');

  simple_field_create_entity_reference ($type, $type->type . '_coach', t ('Coach'), array (hockey_boss_COACH_PROFILE_TYPE));
}

/// Returns an associative array listing the core content types and their constructor functions.
function hockey_boss_types () {
  return array (
    hockey_boss_COACH_PROFILE_TYPE  => 'hockey_boss_create_coach_profile_type',
    hockey_boss_GAME_TYPE           => 'hockey_boss_create_game_type',
    hockey_boss_PLAYER_PROFILE_TYPE => 'hockey_boss_create_player_profile_type',
    hockey_boss_TEAM_PROFILE_TYPE   => 'hockey_boss_create_team_profile_type'
  );
}

/// Creates the core content types.
function hockey_boss_create_types () {
  foreach (array_values (hockey_boss_types ()) as $f) {
    call_user_func ($f);
  }
}

/**
  @brief Accepts a node type and returns the node
    id of every node that has the given type.
  @param $type (string) the node type.
  @return (array) the node ids.
*/
function hockey_boss_get_nodes ($type) {
  $records = db_select   ('{node}', 'tbl')
             ->fields    ('tbl', array ('nid'))
             ->condition ('type', $type)
             ->execute   ();

  $nids = array ();
  foreach ($records as $record) {
    $nids [] = $record->nid;
  }
  return $nids;
}

/**
  @brief Accepts a node type and deletes every node
    that belongs to the given type.
  @param $type (string) the node type.
*/
function hockey_boss_delete_type ($type)  {
  // I. Delete the node type nodes.
  foreach (hockey_boss_get_nodes ($type) as $nid) {
    node_delete ($nid);
  }
  // II. Delete the node type.
  node_type_delete ($type);
}

/// Deletes the core content types.
function hockey_boss_delete_types () {
  foreach (array_keys (hockey_boss_types ()) as $name) {
    hockey_boss_delete_type ($name);
  }
}

// ----- II. User Roles -----

/// The Coach role name. 
define ('hockey_boss_COACH_ROLE', 'hockey_boss_coach_role');

/// The Team Staff role name.
define ('hockey_boss_TEAM_STAFF_ROLE', 'hockey_boss_team_staff_role');

/// The Player role name.
define ('hockey_boss_PLAYER_ROLE', 'hockey_boss_player_role');

/**
  @brief Creates the given user role.
  @param $name (string) the role name.
  @return (boolean) returns true iff the role was 
    created successfully.
  @note This function will display and log an error
    message on failure.
*/
function hockey_boss_create_role ($name) {
  if (!user_role_load_by_name ($name)) {
    if (!user_role_save ((object) array ('name' => $name))) {
      $msg = "Error: an error occured while trying to create the $name role.";
      drupal_set_message ($msg, 'error');
      watchdog (
        'hockey_boss',
        '[hockey_boss_create_role] ' . $msg,
        array (), 
        WATCHDOG_ERROR
      );  
    }   
  }
}

/**
  @brief Assigns the given permissions to the role
    that has the given name.
  @param $name (string) the role name.
  @param $permissions (array) the permission names.
*/
function hockey_boss_grant_permissions ($name, $permissions) {
  $role = user_role_load_by_name ($name);
  user_role_grant_permissions ($role->rid, $permissions);
}

/**
  @brief Returns a list of the core user roles and 
    their permissions.
  @return (array) the core user roles.
*/
function hockey_boss_roles () {
  return array (
    hockey_boss_COACH_ROLE => array (),
    hockey_boss_TEAM_STAFF_ROLE => array (),
    hockey_boss_PLAYER_ROLE => array ()
  );
}

/**
  @brief Creates the core user roles and assigns
    them their default permissions.
*/
function hockey_boss_create_roles () {
  foreach (hockey_boss_roles () as $name => $permissions) {
    // I. create the user role.
    hockey_boss_create_role ($name);

    // II. assign the default permissions.
    hockey_boss_grant_permissions ($name, $permissions);
  }
}

/// Deletes the core user roles.
function hockey_boss_delete_roles () {
  foreach (array_keys (hockey_boss_roles ()) as $name) {
    // I. delete the user role.
    user_role_delete ($name);
  }
}
