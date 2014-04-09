<?php
/**
  @file
  @brief Defines the functions that create the core
    content types and roles along with their fields
    and basic operations.
*/

// ----- I. Defines the content type names. -----

/// Defines the club content type's name.
define ('soccer_boss_CLUB_TYPE', 'soccer_boss_club');

/// Defines the game content type's name.
define ('soccer_boss_GAME_TYPE', 'soccer_boss_game');

/// Defines the league content type's name.
define ('soccer_boss_LEAGUE_TYPE', 'soccer_boss_league');

/// Defines the player content type's name.
define ('soccer_boss_PLAYER_TYPE', 'soccer_boss_player');

/// Defines the team staff's type's name.
define ('soccer_boss_STAFF_TYPE', 'soccer_boss_staff');

/// Defines the team content type's name.
define ('soccer_boss_TEAM_TYPE', 'soccer_boss_team');
 
// ----- I.A. Define the stat type names. -----

/// Defines the player fitness stat (PFS) type's name.
define ('soccer_boss_PFS_TYPE', 'soccer_boss_pfs');

/// Defines the player game stat (PGS) type's name.
define ('soccer_boss_PGS_TYPE', 'soccer_boss_pgs');

/// Defines the player performance stat (PPS) type's name.
define ('soccer_boss_PPS_TYPE', 'soccer_boss_pps');

/// Defines the team stat (TS) type's name.
define ('soccer_boss_TS_TYPE', 'soccer_boss_ts');

// ----- II. Defines the vocabularies -----

/// Returns an array listing the Player vocabularies.
function soccer_boss_vocabularies () {
  global $base_url;

  return array (
    'soccer_boss_age' => array (
      'name'         => 'Age Group',
      'machine_name' => 'soccer_boss_age',
      'description'  => 'The set of recognized age groups.'
    ),
    'soccer_boss_division' => array (
      'name'         => 'Division',
      'machine_name' => 'soccer_boss_division',
      'description'  => 'The set of league divisions.'
    ),
    'soccer_boss_gender' => array (
      'name'         => 'Gender',
      'machine_name' => 'soccer_boss_gender',
      'description'  => 'The set of genders.',
      'url'          => url (drupal_get_path ('module', 'soccer_boss') . '/data/genders.csv', array ('absolute' => true))
    ),
    'soccer_boss_player_position' => array (
      'name'         => 'Player Position',
      'machine_name' => 'soccer_boss_player_position',
      'description'  => 'The set of possible player positions.',
      'url'          => url (drupal_get_path ('module', 'soccer_boss') . '/data/player_positions.csv', array ('absolute' => true))
    ),
    'soccer_boss_staff_role' => array (
      'name'         => 'Staff Role',
      'machine_name' => 'soccer_boss_staff_role',
      'description'  => 'The set of possible staff member roles.',
      'url'          => url (drupal_get_path ('module', 'soccer_boss') . '/data/staff_roles.csv', array ('absolute' => true))
  ));
}

/**
  @brief imports the taxonomy terms listed in the
    CSV file that is referenced by the given url
    into the vocabulary that has the given vid.
  @param $vid (integer) the vocabulary id.
  @param $url (string) the CSV file url.
  @return (array) an error description or nothing.
*/
function soccer_boss_import_terms ($vid, $url) {
  module_load_include ('inc', 'taxonomy_csv', 'import/taxonomy_csv.import.api');

  return taxonomy_csv_import (array (
    'import_format' => 'tree',
    'translate_by' => 'name',
    'translate_languages' => array (), 
    'keep_order' => 0,
    'source_choice' => 'url',
    'filter_format' => 'plain_text',
    'filter_format_custom' => 'none',
    'check_line' => 1,
    'check_utf8' => 1,
    'locale_custom' => '', 
    'vocabulary_target' => 'existing',
    'vocabulary_id' => $vid,
    'i18n_mode' => 0,
    'language' => 'und',
    'delete_terms' => 1,
    'check_hierarchy' => 1,
    'set_hierarchy' => 2,
    'update_or_ignore' => 'update',
    'result_stats' => 'result_stats',
    'result_terms' => 'result_terms',
    'result_level' => 'notices',
    'result_type' => 'by_message',
    'delimiter' => ',',
    'enclosure' => '"',
    'vocabulary_language' => 'und',
    'url' => $url,
    'check_options' => '', 
    'result_display' => 1
  )); 
}

/**
  @brief Creates a new vocabulary.
  @param $vocab_array (array) a vocabulary array.
*/
function soccer_boss_create_vocabulary ($vocab_array) {
  if (!taxonomy_vocabulary_machine_name_load ($vocab_array ['machine_name'])) {
    // I. create the vocabulary.
    taxonomy_vocabulary_save ((object) $vocab_array);
    
    // II. import the terms if given.
    if ($vocab_array ['url']) {
      $vocab = taxonomy_vocabulary_machine_name_load ($vocab_array ['machine_name']);
      soccer_boss_import_terms ($vocab->vid, $vocab_array ['url']);
    }
  }
}

/// Creates the core vocabularies.
function soccer_boss_create_vocabularies () {
  foreach (soccer_boss_vocabularies () as $vocab) {
    soccer_boss_create_vocabulary ($vocab);
  }
}

/// Deletes the core vocabularies.
function soccer_boss_delete_vocabularies () {
  foreach (array_keys (soccer_boss_vocabularies ()) as $machine_name) {  
    $vocab = taxonomy_vocabulary_machine_name_load ($machine_name);
    taxonomy_vocabulary_delete ($vocab->vid);
  }
}

// ----- III. Defines the content types. -----

/// Defines the Club content type.
function soccer_boss_club_content_type () {
  $t = get_t ();
  return node_type_set_defaults (
    array (
      'type'        => soccer_boss_CLUB_TYPE,
      'name'        => $t ('Club'),
      'description' => $t ('Represents amateur soccer clubs.'),
      'help'        => $t ('Represents amateur soccer clubs.'),
      'has_title'   => true,
      'base'        => 'node_content',
      'custom'      => false,
      'modified'    => true,
      'locked'      => true,
      'module'      => 'soccer_boss'
  ));
}

/// Defines the Game content type.
function soccer_boss_game_content_type () {
  $t = get_t (); 
  return node_type_set_defaults (
    array (
      'type'        => soccer_boss_GAME_TYPE,
      'name'        => $t ('Game'),
      'description' => $t ('Represents amateur soccer matches.'),
      'help'        => $t ('Represents amateur soccer matches.'),
      'has_title'   => true,
      'base'        => 'node_content',
      'custom'      => false,
      'modified'    => true,
      'locked'      => true,
      'module'      => 'soccer_boss'
  )); 
}

/// Defines the League content type.
function soccer_boss_league_content_type () {
  $t = get_t (); 
  return node_type_set_defaults (
    array (
      'type'        => soccer_boss_LEAGUE_TYPE,
      'name'        => $t ('League'),
      'description' => $t ('Represents amateur soccer leagues.'),
      'help'        => $t ('Represents amateur soccer leagues.'),
      'has_title'   => true,
      'base'        => 'node_content',
      'custom'      => false,
      'modified'    => true,
      'locked'      => true,
      'module'      => 'soccer_boss'
  )); 
}

/// Defines the Player content type.
function soccer_boss_player_content_type () {
  $t = get_t (); 
  return node_type_set_defaults (
    array (
      'type'        => soccer_boss_PLAYER_TYPE,
      'name'        => $t ('Player'),
      'description' => $t ('Represents amateur soccer players.'),
      'help'        => $t ('Represents amateur soccer players.'),
      'has_title'   => true,
      'base'        => 'node_content',
      'custom'      => false,
      'modified'    => true,
      'locked'      => true,
      'module'      => 'soccer_boss'
  )); 
}

/// Defines the Staff content type.
function soccer_boss_staff_content_type () {
  $t = get_t ();
  return node_type_set_defaults (
    array (
      'type'        => soccer_boss_STAFF_TYPE,
      'name'        => $t ('Staff'),
      'description' => $t ('Represents team staff members.'),
      'help'        => $t ('Represents team staff members.'),
      'has_title'   => true,
      'base'        => 'node_content',
      'custom'      => false,
      'modified'    => true,
      'locked'      => true,
      'module'      => 'soccer_boss'
  ));
}

/// Defines the Team content type.
function soccer_boss_team_content_type () {
  $t = get_t (); 
  return node_type_set_defaults (
    array (
      'type'        => soccer_boss_TEAM_TYPE,
      'name'        => $t ('Team'),
      'description' => $t ('Represents amateur soccer teams.'),
      'help'        => $t ('Represents amateur soccer teams.'),
      'has_title'   => true,
      'base'        => 'node_content',
      'custom'      => false,
      'modified'    => true,
      'locked'      => true,
      'module'      => 'soccer_boss'
  )); 
}

// ----- III.A. Defines the Stat content types.

/// Defines the Player Fitness Stat (PFS) type.
function soccer_boss_pfs_content_type () {
  $t = get_t (); 
  return node_type_set_defaults (
    array (
      'type'        => soccer_boss_PFS_TYPE,
      'name'        => $t ('Player Fitness Stat'),
      'description' => $t ('Represents player fitness stats.'),
      'help'        => $t ('Represents player fitness stats.'),
      'has_title'   => true,
      'base'        => 'node_content',
      'custom'      => false,
      'modified'    => true,
      'locked'      => true,
      'module'      => 'soccer_boss'
  )); 
}

/// Defines the Player Game Stat (PGS) type.
function soccer_boss_pgs_content_type () {
  $t = get_t (); 
  return node_type_set_defaults (
    array (
      'type'        => soccer_boss_PGS_TYPE,
      'name'        => $t ('Player Game Stat'),
      'description' => $t ('Represents player game stats.'),
      'help'        => $t ('Represents player game stats.'),
      'has_title'   => true,
      'base'        => 'node_content',
      'custom'      => false,
      'modified'    => true,
      'locked'      => true,
      'module'      => 'soccer_boss'
  )); 
}

/// Defines the Player Performance Stat (PPS) type.
function soccer_boss_pps_content_type () {
  $t = get_t (); 
  return node_type_set_defaults (
    array (
      'type'        => soccer_boss_PPS_TYPE,
      'name'        => $t ('Player Performance Stat'),
      'description' => $t ('Represents player performance stats.'),
      'help'        => $t ('Represents player performance stats.'),
      'has_title'   => true,
      'base'        => 'node_content',
      'custom'      => false,
      'modified'    => true,
      'locked'      => true,
      'module'      => 'soccer_boss'
  )); 
}

/// Defines the Team Stat (TS) type.
function soccer_boss_ts_content_type () {
  $t = get_t (); 
  return node_type_set_defaults (
    array (
      'type'        => soccer_boss_TS_TYPE,
      'name'        => $t ('Team Stat'),
      'description' => $t ('Represents team stats.'),
      'help'        => $t ('Represents team stats.'),
      'has_title'   => true,
      'base'        => 'node_content',
      'custom'      => false,
      'modified'    => true,
      'locked'      => true,
      'module'      => 'soccer_boss'
  )); 
}

/// Returns the content type definitions array.
function soccer_boss_content_types () {
  return array (
    soccer_boss_CLUB_TYPE => array (
      'type'       => soccer_boss_club_content_type (),
      'body_label' => 'Overview',
      'fields'     => array (
        'soccer_boss_club_league' => 'soccer_boss_create_club_league_field'
    )),
    soccer_boss_GAME_TYPE => array (
      'type'       => soccer_boss_game_content_type (),
      'body_label' => 'Commentary',
      'fields'     => array (
        'soccer_boss_game_teams' => 'soccer_boss_create_game_teams_field',
        'soccer_boss_game_date'  => 'soccer_boss_create_game_date_field',
        'soccer_boss_game_score' => 'soccer_boss_create_game_score_field',
    )),
    soccer_boss_LEAGUE_TYPE => array (
      'type'       => soccer_boss_league_content_type (),
      'body_label' => 'Overview',
      'fields'     => array ()
    ),
    soccer_boss_PLAYER_TYPE => array (
      'type'       => soccer_boss_player_content_type (),
      'body_label' => 'Bio',
      'fields'     => array (
        'soccer_boss_player_photo'      => 'soccer_boss_create_player_photo_field',
        'soccer_boss_player_first_name' => 'soccer_boss_create_player_first_name_field',
        'soccer_boss_player_last_name'  => 'soccer_boss_create_player_last_name_field',
        'soccer_boss_player_team'       => 'soccer_boss_create_player_team_field',
        'soccer_boss_player_position'   => 'soccer_boss_create_player_position_field',
        'soccer_boss_player_height'     => 'soccer_boss_create_player_height_field',
        'soccer_boss_player_weight'     => 'soccer_boss_create_player_weight_field',
        'soccer_boss_player_birth_date' => 'soccer_boss_create_player_birth_date_field'
    )),
    soccer_boss_STAFF_TYPE => array (
      'type'       => soccer_boss_staff_content_type (),
      'body_label' => 'Bio',
      'fields'     => array (
        'soccer_boss_staff_photo'      => 'soccer_boss_create_staff_photo_field',
        'soccer_boss_staff_first_name' => 'soccer_boss_create_staff_first_name_field',
        'soccer_boss_staff_last_name'  => 'soccer_boss_create_staff_last_name_field',
        'soccer_boss_staff_team'       => 'soccer_boss_create_staff_team_field',
        'soccer_boss_staff_role'       => 'soccer_boss_create_staff_role_field'
    )),
    soccer_boss_TEAM_TYPE => array (
      'type'       => soccer_boss_team_content_type (),
      'body_label' => 'Overview',
      'fields'     => array (
        'soccer_boss_team_age'      => 'soccer_boss_create_team_age_field',
        'soccer_boss_team_club'     => 'soccer_boss_create_team_club_field',
        'soccer_boss_team_division' => 'soccer_boss_create_team_division_field',
        'soccer_boss_team_gender'   => 'soccer_boss_create_team_gender_field',
        'soccer_boss_team_logo'     => 'soccer_boss_create_team_logo_field'
    )),
    soccer_boss_PFS_TYPE => array (
      'type'       => soccer_boss_pfs_content_type (),
      'body_label' => false,
      'fields'     => array (
        'soccer_boss_pfs_player' => 'soccer_boss_create_pfs_player_field',
        'soccer_boss_pfs_value'  => 'soccer_boss_create_pfs_value_field',
        'soccer_boss_pfs_date'   => 'soccer_boss_create_pfs_date_field'
    )),
    soccer_boss_PGS_TYPE => array (
      'type'       => soccer_boss_pgs_content_type (),
      'body_label' => false,
      'fields'     => array (
        'soccer_boss_pgs_player' => 'soccer_boss_create_pgs_player_field',
        'soccer_boss_pgs_game'   => 'soccer_boss_create_pgs_game_field',
        'soccer_boss_pgs_value'  => 'soccer_boss_create_pgs_value_field'
    )),
    soccer_boss_PPS_TYPE => array (
      'type'       => soccer_boss_pps_content_type (),
      'body_label' => false,
      'fields'     => array (
        'soccer_boss_pps_player' => 'soccer_boss_create_pps_player_field',
        'soccer_boss_pps_value'  => 'soccer_boss_create_pps_value_field',
        'soccer_boss_pps_year'   => 'soccer_boss_create_pps_year_field'
    )),
    soccer_boss_TS_TYPE => array (
      'type'       => soccer_boss_ts_content_type (),
      'body_label' => false,
      'fields'     => array (
        'soccer_boss_ts_team'  => 'soccer_boss_create_ts_team_field',
        'soccer_boss_ts_value' => 'soccer_boss_create_ts_value_field',
        'soccer_boss_ts_year'  => 'soccer_boss_create_ts_year_field'
    ))
  );
}

/// Creates the content types.
function soccer_boss_create_content_types () {
  module_load_include ('inc', 'soccer_boss', 'soccer_boss.field');

  foreach (soccer_boss_content_types () as $name => $definition) {
    // I. create the content type.
    node_type_save ($definition ['type']);

    // II. create the fields.
    if ($definition ['body_label']) {
      node_add_body_field ($definition ['type'], $definition ['body_label']);
    }
    foreach ($definition ['fields'] as $field_name => $field_hook) {
      call_user_func ($field_hook);
    }
  }
}

/// Deletes the content types.
function soccer_boss_delete_content_types () {
  foreach (soccer_boss_content_types () as $name => $definition) {
    // I. delete the nodes.
    soccer_boss_delete_nodes ($name);

    // II. delete the fields.
    foreach (array_keys ($definition ['fields']) as $field_name) {
      field_delete_field ($field_name);
    }

    // III. delete the type.
    node_type_delete ($name);
  }
}

// ----- IV. Defines the basic node operations. -----

/**
  @brief Returns the node id of every node that has the given type.
  @param $type (string) the node type.
  @return (array) the node ids.
*/
function soccer_boss_get_nodes ($type) {
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
  @brief Deletes every node that has the given type.
  @param $type (string) the node type.
*/
function soccer_boss_delete_nodes ($type) {
  foreach (soccer_boss_get_nodes ($type) as $nid) {
    node_delete ($nid);
  }
}

// ----- V. Defines the core user roles. -----

/// Define the Player role name.
define ('soccer_boss_PLAYER_ROLE', 'soccer_boss_player');

/// Define the Team Staff role name.
define ('soccer_boss_TEAM_STAFF_ROLE', 'soccer_boss_team_staff');

/// Define the Club Staff role name.
define ('soccer_boss_CLUB_STAFF_ROLE', 'soccer_boss_club_staff');

/// Defines the Director role name.
define ('soccer_boss_DIRECTOR_ROLE', 'soccer_boss_director');

/**
  @brief Creates the given user role.
  @param $name (string) the role name.
  @return (boolean) returns true iff the role was 
    created successfully.
  @note This function will display and log an error
    message on failure.
*/
function soccer_boss_create_role ($name) {
  if (!user_role_load_by_name ($name)) {
    if (!user_role_save ((object) array ('name' => $name))) {
      $msg = "Error: an error occured while trying to create the $name role.";
      drupal_set_message ($msg, 'error');
      watchdog (
        'soccer_boss',
        '[soccer_boss_create_role] ' . $msg,
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
function soccer_boss_grant_permissions ($name, $permissions) {
  $role = user_role_load_by_name ($name);
  user_role_grant_permissions ($role->rid, $permissions);
}

/**
  @brief Returns a list of the core user roles and
    their permissions.
  @return (array) the core user roles.
*/
function soccer_boss_roles () {
  return array (
    soccer_boss_PLAYER_ROLE => array (
      'create '     . soccer_boss_PLAYER_TYPE            . ' content', 
      'edit own '   . soccer_boss_PLAYER_TYPE            . ' content', 
      'delete own ' . soccer_boss_PLAYER_TYPE            . ' content',
      'create '     . soccer_boss_PPS_TYPE               . ' content',
      'edit any '   . soccer_boss_PPS_TYPE               . ' content',
      'delete any ' . soccer_boss_PPS_TYPE               . ' content',
    ),
    soccer_boss_TEAM_STAFF_ROLE => array (
      'create '     . soccer_boss_GAME_TYPE              . ' content',
      'edit any '   . soccer_boss_GAME_TYPE              . ' content',
      'delete any ' . soccer_boss_GAME_TYPE              . ' content',
      'create '     . soccer_boss_PLAYER_TYPE            . ' content', 
      'edit any '   . soccer_boss_PLAYER_TYPE            . ' content', 
      'delete any ' . soccer_boss_PLAYER_TYPE            . ' content', 
      'create '     . soccer_boss_STAFF_TYPE             . ' content',
      'edit own '   . soccer_boss_STAFF_TYPE             . ' content',
      'delete own ' . soccer_boss_STAFF_TYPE             . ' content',
      'create '     . soccer_boss_TEAM_TYPE              . ' content',
      'edit own '   . soccer_boss_TEAM_TYPE              . ' content',
      'delete own ' . soccer_boss_TEAM_TYPE              . ' content',
      'create '     . soccer_boss_PGS_TYPE               . ' content',
      'edit any '   . soccer_boss_PGS_TYPE               . ' content',
      'delete any ' . soccer_boss_PGS_TYPE               . ' content',
      'create '     . soccer_boss_PPS_TYPE               . ' content',
      'edit any '   . soccer_boss_PPS_TYPE               . ' content',
      'delete any ' . soccer_boss_PPS_TYPE               . ' content',
      'create '     . soccer_boss_TS_TYPE               . ' content',
      'edit any '   . soccer_boss_TS_TYPE               . ' content',
      'delete any ' . soccer_boss_TS_TYPE               . ' content',
    ),
    soccer_boss_CLUB_STAFF_ROLE => array (
      'create '     . soccer_boss_CLUB_TYPE              . ' content',
      'edit any '   . soccer_boss_CLUB_TYPE              . ' content',
      'delete any ' . soccer_boss_CLUB_TYPE              . ' content',
      'create '     . soccer_boss_GAME_TYPE              . ' content',
      'edit any '   . soccer_boss_GAME_TYPE              . ' content',
      'delete any ' . soccer_boss_GAME_TYPE              . ' content',
      'create '     . soccer_boss_PLAYER_TYPE            . ' content', 
      'edit any '   . soccer_boss_PLAYER_TYPE            . ' content', 
      'delete any ' . soccer_boss_PLAYER_TYPE            . ' content', 
      'create '     . soccer_boss_STAFF_TYPE             . ' content',
      'edit own '   . soccer_boss_STAFF_TYPE             . ' content',
      'delete own ' . soccer_boss_STAFF_TYPE             . ' content',
      'create '     . soccer_boss_TEAM_TYPE              . ' content',
      'edit own '   . soccer_boss_TEAM_TYPE              . ' content',
      'delete own ' . soccer_boss_TEAM_TYPE              . ' content',
      'create '     . soccer_boss_PGS_TYPE               . ' content',
      'edit any '   . soccer_boss_PGS_TYPE               . ' content',
      'delete any ' . soccer_boss_PGS_TYPE               . ' content',
      'create '     . soccer_boss_PPS_TYPE               . ' content',
      'edit any '   . soccer_boss_PPS_TYPE               . ' content',
      'delete any ' . soccer_boss_PPS_TYPE               . ' content',
      'create '     . soccer_boss_TS_TYPE               . ' content',
      'edit any '   . soccer_boss_TS_TYPE               . ' content',
      'delete any ' . soccer_boss_TS_TYPE               . ' content',
    ),
    soccer_boss_DIRECTOR_ROLE => array (
    )
  );
}


/**
  @brief Creates the core user roles and assigns
    them their default permissions.
*/
function soccer_boss_create_roles () {
  foreach (soccer_boss_roles () as $name => $permissions) {
    // I. create the user role.
    soccer_boss_create_role ($name);

    // II. assign the default permissions.
    soccer_boss_grant_permissions ($name, $permissions);
  }
}

/// Deletes the core user roles.
function soccer_boss_delete_roles () {
  foreach (array_keys (soccer_boss_roles ()) as $name) {
    // I. delete the user role.
    user_role_delete ($name);
  }
}

// ----- VI. Block visibility functions -----

/**
  @brief Restricts the given block to the given content types.
  @param $delta (string) the block delta.
  @param $type (string) the content type.
  @param $module (string) the module that defined the block.
*/
function soccer_boss_set_block_visibility ($delta, $type, $module) {
  db_insert ('block_node_type')
    ->fields (array ('type', 'module', 'delta'))
    ->values (array (
        'type'   => $type,
        'module' => $module,
        'delta'  => $delta
      ))
    ->execute ();
}

/**
  @brief Returns an array listing those blocks
    whose visibility settings are overriden by this
    module.
  @return (array) the settings array.
*/
function soccer_boss_block_settings () {
  return array (
    'club_teams-block' => array (
      'type'   => soccer_boss_CLUB_TYPE,
      'region' => 'content',
      'module' => 'views'
    ),
    'league_clubs-block' => array (
      'type'   => soccer_boss_LEAGUE_TYPE,
      'region' => 'content',
      'module' => 'views'
    ),
    'player_vital_stats-block' => array (
      'type'   => soccer_boss_PLAYER_TYPE,
      'region' => 'content',
      'module' => 'views',
      'weight' => 1
    ),
    'team_games-block' => array (
      'type'   => soccer_boss_TEAM_TYPE,
      'region' => 'content',
      'module' => 'views',
      'weight' => 2
    ),
    'team_roster-block' => array (
      'type'   => soccer_boss_TEAM_TYPE,
      'region' => 'content',
      'module' => 'views',
      'weight' => 4
    ),
    'team_staff-block' => array (
      'type'   => soccer_boss_TEAM_TYPE,
      'region' => 'content',
      'module' => 'views',
      'weight' => 3
    ),
    'team_vitals-block' => array (
      'type'   => soccer_boss_TEAM_TYPE,
      'region' => 'content',
      'module' => 'views',
      'weight' => 1
  ));
}

/**
  @brief Sets the default visibilities for the core blocks.
  @see soccer_boss_block_info_alter ().
*/
function soccer_boss_set_block_visibilities () {
  foreach (soccer_boss_block_settings () as $delta => $settings) {
    soccer_boss_set_block_visibility (
      $delta,
      $settings ['type'],
      $settings ['module']
    );
  }
}

/// Unsets the visibility settings for the core blocks.
function soccer_boss_clear_block_visibilities () {
  foreach (soccer_boss_block_settings () as $delta => $settings) {
    db_delete ('block_node_type')
      ->condition ('delta', $delta) 
      ->condition ('module', $settings ['module'])
      ->execute   ();
  }
}

// ----- VII. Miscellaneous -----

/**
  @brief Returns the ids of those entities that
    reference the given entity using the given
    entity reference field.
  @param $id (integer) the referenced entity id.
  @param $type (string) the referenced entity type.
  @param $field_name (string) the entity reference field name.
  @return (array) the referencing entity ids.
*/
function soccer_boss_get_references ($id, $type, $field_name) {
  $records = db_select     ('{field_data_' . $field_name . '}', 'tbl')
               ->fields    ('tbl', array ('entity_id'))
               ->condition ('entity_type', $type)
               ->condition ($field_name . '_target_id', $id)
               ->execute   ();

  $ids = array ();
  foreach ($records as $record) {
    $ids [] = $record->entity_id;
  }
  watchdog (
    'soccer_boss',
    '[soccer_boss_get_references] field name: @field_name ids: <pre>@ids</pre>',
    array (
      '@field_name' => $field_name,
      '@ids'        => print_r ($ids, true)
  ));
  return $ids;
}

/**
  @brief Accepts a team node id and returns the
    node ids of the players that are on the team.
  @param $team_nid (integer) the team node id.
  @return (array) the player node ids.
*/
function soccer_boss_get_team_players ($team_nid) {
  return node_load_multiple (
    soccer_boss_get_references ($team_nid, 'node', 'soccer_boss_player_team'));
}

/**
  @brief Returns the Player Fitness Stats
    associated with the given player.
  @param $nid (integer) the player node id.
  @return (array) the Player Fitness Stat nodes.
*/
function soccer_boss_get_pfs_nodes ($nid) {
  return node_load_multiple (
    soccer_boss_get_references ($nid, 'node', 'soccer_boss_pfs_player'));
}

/**
  @brief Returns the Player Game Stats associated
    with the given player.
  @param $player_nid (integer) the player node id.
  @param $game_nid (integer) the game node id.
  @return (array) the Player Game Stat nodes.
*/
function soccer_boss_get_pgs_nodes ($player_nid, $game_nid = null) {
  $stats = node_load_multiple (
    soccer_boss_get_references ($player_nid, 'node', 'soccer_boss_pgs_player'));

  $filtered_stats = array ();
  if (is_null ($game_nid)) {
    $filtered_stats = $stats;
  } else {
    foreach ($stats as $stat) {
      if ($game_nid == $stat->soccer_boss_pgs_game ['und'][0]['target_id']) {
        $filtered_stats [] = $stat;
      }
    }
  }
  return $filtered_stats;
}

/**
  @brief Returns the Player Performance Stats
    associated with the given player.
  @param $nid (integer) the player node id.
  @return (array) the Player Performance Stat nodes.
*/
function soccer_boss_get_pps_nodes ($nid) {
  return node_load_multiple (
    soccer_boss_get_references ($nid, 'node', 'soccer_boss_pps_player'));
}

/**
  @brief Returns an array that represents a derived
    player performance stat - I.E. a stat whose
    value has been derived by adding up game stats.
*/
function soccer_boss_get_derived_pps ($player_nid, $stat_name, $year) {
  $game_stats = array ();
  foreach (soccer_boss_get_pgs_nodes ($player_nid) as $game_stat) {
    if ($game_stat->title != $stat_name) { continue; }

    $game = node_load ($game_stat->soccer_boss_pgs_game ['und'][0]['target_id']);
    if (!$game) { continue; }

    $game_year = date ('Y', strtotime ($game->soccer_boss_game_date ['und'][0]['value']));
    if ($game_year == $year) {
      $game_stats [] = $game_stat;
    }
  }

  $derived_value = empty ($game_stats) ? null : 0;
  foreach ($game_stats as $game_stat) {
    $derived_value += $game_stat->soccer_boss_pgs_value ['und'][0]['value'];
  }

  return array (
    'player' => $player_nid,
    'name'   => $stat_name,
    'value'  => $derived_value,
    'year'   => $year
  );
}

/**
  @brief Accepts a player node id, fitness stat
    name, and a year, and returns the player
    fitness stat for the given year.
  @param $player_nid (integer) the player node id.
  @param $stat_name (string) the stat name.
  @param $year (string) the year.
*/
function soccer_boss_get_pfs ($player_nid, $stat_name, $year) {
  $stat_nodes = soccer_boss_get_pfs_nodes ($player_nid);
  foreach ($stat_nodes as $stat_node) {
    $stat_year = date ('Y', strtotime ($stat_node->soccer_boss_pfs_date ['und'][0]['value']));
    if ($stat_node->title == $stat_name && $stat_year == $year) {
      return array (
        'player' => $player_nid,
        'name'   => $stat_name,
        'value'  => $stat_node->soccer_boss_pfs_value ['und'][0]['value'],
        'year'   => $year
      );
    }
  }
  return null;
}

/**
  @brief Accepts a player node id, game stat
    name, and a year, and returns the player
    game stat for the given year - I.E. the total
    for all games played in the given year.
  @param $player_nid (integer) the player node id.
  @param $stat_name (string) the stat name.
  @param $year (string) the year.
*/
function soccer_boss_get_pgs ($player_nid, $stat_name, $year) {
  $stat_nodes = soccer_boss_get_pgs_nodes ($player_nid);
  $value = null;
  foreach ($stat_nodes as $stat_node) {
    if ($stat_node->title == $stat_name) {
      $game_node = node_load ($stat_node->soccer_boss_pgs_game ['und'][0]['target_id']);
      $stat_year = date ('Y', strtotime ($game_node->soccer_boss_game_date ['und'][0]['value']));
      if ($stat_year == $year) {
        if (is_null ($value)) {
          $value = 0;
        }
        $value += $stat_node->soccer_boss_pgs_value ['und'][0]['value'];
      }
    }
  }
  return is_null ($value) ? null :
    array (
      'player' => $player_nid,
      'name'   => $stat_name,
      'value'  => $value,
      'year'   => $year
    );
}

/**
  @brief Accepts a player node id, performance stat
    name, and a year, and returns the player
    performance stat for the given year. If a pps
    node does not exist, this function will derive
    the value from the game stats.
  @param $player_nid (integer) the player node id.
  @param $stat_name (string) the stat name.
  @param $year (string) the year.
*/
function soccer_boss_get_pps ($player_nid, $stat_name, $year) {
  $stats = soccer_boss_get_pps_nodes ($player_nid);
  foreach ($stats as $stat) {
    if (
      $stat->soccer_boss_pps_year ['und'][0]['value'] == $year &&
      $stat->title == $stat_name 
    ) {
      return array (
        'player' => $player_nid,
        'name'   => $stat_name,
        'value'  => $stat->soccer_boss_pps_value ['und'][0]['value'],
        'year'   => $year
      );
    }
  }
  return soccer_boss_get_derived_pps ($player_nid, $stat_name, $year);
}

/**
  @brief Accepts a team node id, a game stat name,
    and a year, and returns the team's totals for
    the game stat during the given year.
  @param $team_nid (integer) the team node id.
  @param $stat_name (string) the stat name.
  @param $year (string) the year.
  @return (float) the total value or null if the
    stat does not exist.
*/
function soccer_boss_get_tgs ($team_nid, $stat_name, $year) {
  $value = null;
  foreach (soccer_boss_get_team_players ($team_nid) as $player) {
    $pgs = soccer_boss_get_pgs ($player->nid, $stat_name, $year);
    if (!is_null ($pgs ['value'])) {
      if (is_null ($value)) {
        $value = 0;
      }
      $value += $pgs ['value'];
    }
  }
  return $value;
}

/**
  @brief Accepts a team node id, a player
    performance stat name, and a year, and returns
    the average stat value for the players on the
    given team during the given year.
  @param $team_nid (integer) the team node id.
  @param $stat_name (string) the stat name.
  @param $year (string) the year.
  @return (float) the average value or null if the
    stat does not exist.
  @note TAPS - Team Average Performance Stats
*/
function soccer_boss_get_taps ($team_nid, $stat_name, $year) {
  $value = null;
  $num_players = 0;
  foreach (soccer_boss_get_team_players ($team_nid) as $player) {
    $pfs = soccer_boss_get_pfs ($player->nid, $stat_name, $year);
    if (!is_null ($pfs ['value'])) {
      if (is_null ($value)) {
        $value = 0;
      }
      $value += $pfs ['value'];
      $num_players ++;
    }
  }
  if (!is_null ($value)) {
    $value = $value / $num_players;
  }
  return $value;
}