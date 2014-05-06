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
  @param $name (string) the internal type name.
  @param $label (string) the human-readable type name.
  @param $description (string) a description of the type.
  @return (object) a content type.
*/
function hockey_boss_get_type ($name, $label, $description) {
  $t = get_t (); 
  return node_type_set_defaults (
    array (
      'type'        => $name,
      'name'        => $t ($label),
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

/// Defines the Coach Profile content type.
function hockey_boss_coach_profile_type () {
  return hockey_boss_get_type (hockey_boss_COACH_PROFILE_TYPE, 'Coach Profile', 'Represents coach profiles.');
}

/// Defines the Game type.
function hockey_boss_game_type () {
  return hockey_boss_get_type (hockey_boss_GAME_TYPE, 'Game', 'Represents matches.');
}

/// Defines the Player Profile content type.
function hockey_boss_player_profile_type () {
  return hockey_boss_get_type (hockey_boss_PLAYER_PROFILE_TYPE, 'Player Profile', 'Represents amateur hockey players.');
}

/// Defines the Team Profile content type.
function hockey_boss_team_profile_type () {
  return hockey_boss_get_type (hockey_boss_TEAM_PROFILE_TYPE, 'Team Profile', 'Represents amateur hockey teams.');
}

/// Returns the core content types.
function hockey_boss_types () {
  return array (
    hockey_boss_coach_profile_type (),
    hockey_boss_game_type (),
    hockey_boss_player_profile_type (),
    hockey_boss_team_profile_type ()
  );
}

/// Creates the core content types.
function hockey_boss_create_types () {
  foreach (hockey_boss_types () as $type) {
    node_type_save ($type);
  }
}

/**
  @brief Accepts a player node and returns the user id of the user associated with it.
  @return (int) the user id.
*/
function hockey_boss_get_player_user_uid ($node) {
  return simple_field_get_relation_source_entity_id ($node, hockey_boss_PLAYER_PROFILE_TYPE . '_user');
}

/**
  @brief Accepts a player node and returns its team node id.
  @return (int) the team node id.
*/
function hockey_boss_get_player_team_nid ($node) {
  return simple_field_get_relation_target_entity_id ($node, hockey_boss_PLAYER_PROFILE_TYPE . '_team');
}

/**
  @brief Accepts a team node and returns its coach node id.
  @return (int) the coach node id.
*/
function hockey_boss_get_team_coach_nid ($node) {
  return simple_field_get_relation_source_entity_id ($node, hockey_boss_TEAM_PROFILE_TYPE . '_coach');
}

/**
  @brief Accepts a coach node and returns its user id.
  @return (int) the coach user id.
*/
function hockey_boss_get_coach_user_uid ($node) {
  return simple_field_get_relation_source_entity_id ($node, hockey_boss_COACH_PROFILE_TYPE . '_user');
}

/**
  @brief Accepts a game node and returns its date.
  @return (string) the date.
*/
function hockey_boss_get_game_date ($node) {
  return $node ? $node->{hockey_boss_GAME_TYPE . '_date'}['und'][0]['value'] : null;
}

/**
  @brief Accepts a game node and returns the node id of the teams.
  @return (array) the team node ids.
*/
function hockey_boss_get_game_team_nids ($node) {
  return simple_field_get_entity_reference_target_ids ($node, hockey_boss_GAME_TYPE . '_teams');
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
  @brief Returns the node ids of the games ordered by date.
  @return (array) the node ids.
*/
function hockey_boss_get_game_nodes () {
  $game_nodes = array ();
  $game_nids = hockey_boss_get_nodes (hockey_boss_GAME_TYPE);
  foreach ($game_nids as $game_nid) {
    $game_node = node_load ($game_nid);
    $game_date = hockey_boss_get_game_date ($game_node);
    $game_nodes [$game_date] = $game_node;
  }
  ksort ($game_nodes);
  return $game_nodes;
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
  foreach (hockey_boss_types () as $type) {
    hockey_boss_delete_type ($type->type);
  }
}

// ----- II. Relations -----

/// The Coach/Team relation name.
define ('hockey_boss_COACH_TEAM_RELATION', 'hockey_boss_coach_team_relation');

/// The Player/Team relation name.
define ('hockey_boss_PLAYER_TEAM_RELATION', 'hockey_boss_player_team_relation');

/// The User/Coach relation name.
define ('hockey_boss_USER_COACH_RELATION', 'hockey_boss_user_coach_relation');

/// The User/Player relation name.
define ('hockey_boss_USER_PLAYER_RELATION', 'hockey_boss_user_player_relation');

/**
  @brief Defines a relation type.
  @param $name (string) the internal type name.
  @param $label (string) the human-readable type name.
  @param $source_bundles (array) the source bundles.
  @param $directional (boolean) indicates whether
    or not the relation is directional.
  @param $reverse_label (string) the human-readable
    name of the reversed relation. Default $label.
  @param $target_bundles (array) the target bundles.
  @return (relation_type).
*/
function hockey_boss_get_relation ($name, $label, $source_bundles, $directional = false, $reverse_label = null, $target_bundles = array ()) {
  if (is_null ($reverse_label)) { $reverse_label = $label; }
  return relation_type_create (array (
    'relation_type'  => $name,
    'label'          => $label,
    'reverse_label'  => $reverse_label,
    'directional'    => $directional,
    'source_bundles' => $source_bundles,
    'target_bundles' => $target_bundles
  ));
}

/// Defines the coach/team relation.
function hockey_boss_coach_team_relation () {
  return hockey_boss_get_relation (hockey_boss_COACH_TEAM_RELATION, 'Coach Team', array ('node:' . hockey_boss_COACH_PROFILE_TYPE), true, 'Team Coach', array ('node:' . hockey_boss_TEAM_PROFILE_TYPE));
}

/// Defines the player/team relation.
function hockey_boss_player_team_relation () {
  return hockey_boss_get_relation (hockey_boss_PLAYER_TEAM_RELATION, 'Player Team', array ('node:' . hockey_boss_PLAYER_PROFILE_TYPE), true, 'Team Player', array ('node:' . hockey_boss_TEAM_PROFILE_TYPE));
}

/// Defines the user/coach relation.
function hockey_boss_user_coach_relation () {
  return hockey_boss_get_relation (hockey_boss_USER_COACH_RELATION, 'User Coach Profile', array ('user:user'), true, 'Coach Profile User', array ('node:' . hockey_boss_COACH_PROFILE_TYPE)); 
}

/// Defines the user/player relation.
function hockey_boss_user_player_relation () {
  return hockey_boss_get_relation (hockey_boss_USER_PLAYER_RELATION, 'User Player Profile', array ('user:user'), true, 'Player Profile User', array ('node:' . hockey_boss_PLAYER_PROFILE_TYPE));
}

/// Returns the core relations.
function hockey_boss_relations () {
  return array (
    hockey_boss_coach_team_relation (),
    hockey_boss_player_team_relation (),
    hockey_boss_user_coach_relation (),
    hockey_boss_user_player_relation ()
  );
}

/// Creates the core relations.
function hockey_boss_create_relations () {
  foreach (hockey_boss_relations () as $relation) {
    relation_type_save ($relation);
  }
}

/// Deletes the core relations.
function hockey_boss_delete_relations () {
  foreach (hockey_boss_relations () as $relation) {
    relation_type_delete ($relation->relation_type);
  }
}

// ----- III. Create Fields. -----

/// Creates the Coach Profile fields. 
function hockey_boss_create_coach_profile_fields () {
  $type = hockey_boss_coach_profile_type ();
  node_add_body_field ($type, 'Bio');
  simple_field_create_relation_field ($type, $type->type . '_user', t ('User Account'), hockey_boss_USER_COACH_RELATION . ':reverse', 1);
  simple_field_create_relation_field ($type, $type->type . '_team', t ('Team'), hockey_boss_COACH_TEAM_RELATION);
}

/// Creates the Game fields.
function hockey_boss_create_game_fields () {
  $type = hockey_boss_game_type ();
  node_add_body_field ($type, 'Recap');
  simple_field_create_date_field ($type, $type->type . '_date', t ('Date'));
  simple_field_create_textfield ($type, $type->type . '_scores', t ('Scores'));
  simple_field_create_entity_reference ($type, $type->type . '_teams', t ('Teams'), array (hockey_boss_TEAM_PROFILE_TYPE), 'node', 2);
}

/// Creates the Player Profile fields.
function hockey_boss_create_player_profile_fields () {
  $type = hockey_boss_player_profile_type ();
  simple_field_create_relation_field ($type, $type->type . '_user', t ('User Account'), hockey_boss_USER_PLAYER_RELATION . ':reverse', 1);
  simple_field_create_relation_field ($type, $type->type . '_team', t ('Team'), hockey_boss_PLAYER_TEAM_RELATION);
}

/// Creates the Team Profile fields.
function hockey_boss_create_team_profile_fields () {
  $type = hockey_boss_team_profile_type ();
  simple_field_create_relation_field ($type, $type->type . '_coach', t ('Coach'), hockey_boss_COACH_TEAM_RELATION . ':reverse', 1);
  simple_field_create_relation_field ($type, $type->type . '_player', t ('Players'), hockey_boss_PLAYER_TEAM_RELATION . ':reverse');
}

/// Creates the core fields.
function hockey_boss_create_fields () {
  hockey_boss_create_coach_profile_fields ();
  hockey_boss_create_game_fields ();
  hockey_boss_create_player_profile_fields ();
  hockey_boss_create_team_profile_fields ();
}

// ----- IV. User Roles -----

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
