<?php
/**
  @file
  @brief Defines the stat tables.
*/

/// Represents player performance stat tables. 
class PPSTable extends SimpleTable {
  /**
    The node id of the player whose stats are
    displayed in this table.
  */
  public $player_nid = null;

  /// Returns a player performance stat table.
  public function __construct ($player_nid, $current_year) {
    parent::__construct ();
    
    $this->player_nid = $player_nid;

    // set row names.
    $start_year = $current_year - 1;
    for ($year = $start_year; $year <= $current_year; $year ++) {
      $this->row_names [] = $year;
    }

    // set columns.
    $this->columns = $this->create_columns (
      array ('+/-', 'Face-Offs', 'Penalties', 'SAOG', 'Shots Against', 'SOG', 'Shots', 'Assists', 'Goals', 'Year'));

    $editable = $this->editable ();
    $this->submittable       = $editable;
    $this->sortable          = !$editable;
    $this->sort_column_name  = 'Year';
    $this->label_column_name = 'Year';
  }

  /// Returns true iff the current user has permission to edit this table.
  public function editable () {
    module_load_include ('inc', 'hockey_boss');

    global $user;
    $player_node = node_load ($this->player_nid);

    $player_uid = hockey_boss_get_player_user_uid ($player_node);
    $team_node = hockey_boss_get_player_team_node ($player_node);
    $coach_node = is_null ($team_node) ? null : hockey_boss_get_team_coach_node ($team_node);
    $coach_uid = hockey_boss_get_coach_user_uid ($coach_node);

    return user_access ('hockey_boss_stat_edit_all_pps') ||
           ($user->uid == $player_uid && user_access ('hockey_boss_stat_edit_own_pps')) ||
           ($user->uid == $coach_uid  && user_access ('hockey_boss_stat_edit_team_pps'));
  }

  /// Returns the default field values.
  public function get_default_field_value ($column_name, $row_name) {
    module_load_include ('inc', 'stat');
    module_load_include ('inc', 'hockey_boss_stat');

    $field_width = 5;

    $stat_name = $column_name;
    $stat_year = $row_name;

    $pps_query = stat_node_query_create (hockey_boss_stat_PPS_TYPE);
    $pps_query = stat_node_query_add_year_filter ($pps_query, $stat_year);
    $pps_query = stat_node_query_add_title_filter ($pps_query, $stat_name);
    $pps_query = stat_node_query_add_subject_filter ($pps_query, $this->player_nid);
    $stat_nodes = stat_node_query_execute ($pps_query);
    $stat = empty ($stat_nodes) ? null : stat_from_stat_node ($stat_nodes [0]);

    if (is_null ($stat)) {
      $stat = stat_create ($stat_name, hockey_boss_stat_PPS_TYPE, 0, '', "$stat_year-01-01 00:00:00", $this->player_nid);
      $pgs_query = stat_node_query_create (hockey_boss_stat_PGS_TYPE);
      $pgs_query = stat_node_query_add_year_filter ($pgs_query, $stat_year);
      $pgs_query = stat_node_query_add_title_filter ($pgs_query, $stat_name);
      $pgs_query = stat_node_query_add_subject_filter ($pgs_query, $this->player_nid);
      $pgs_nodes = stat_node_query_execute ($pgs_query);

      foreach ($pgs_nodes as $pgs_node) {
        $stat ['value'] += stat_node_get_value ($pgs_node);
      }
    }

    if ($this->editable () && isset ($stat ['nid'])) {
      return array (
        'id'            => null,
        'value'         => $stat ['value'],
        'display_value' => array (
          '#type' => 'container',
          'stat_value_' . $stat ['nid'] => array (
            '#type'               => 'textfield',
            '#default_value'      => $stat ['value'],
            '#size'               => $field_width
      )));
    } else {
      return array (
        'id'            => null,
        'value'         => $stat ['value'],
        'display_value' => $stat ['value']
      );
    }
  }

  /// The submit callback.
  public static function process_table_form ($form, $form_state) {
    module_load_include ('inc', 'stat');
    foreach ($form_state ['input'] as $key => $value) {
      if (strpos ($key, 'stat_value_') === 0) {
        $stat_nid = substr ($key, 11);
        $stat_node = node_load ($stat_nid);
        if (is_null ($stat_node)) {
          continue;
        }
        stat_node_set_value ($stat_node, $value);
        node_save ($stat_node);
      }
    }
    drupal_set_message ('Stats updated.');
  }
}

/// Represents player fitness stat tables.
class PFSTable extends SimpleTable {
  /**
    The node of the player whose stats are displayed
    in this table.
  */
  public $player_node = null;

  /// Returns a player fitness stat table.
  public function __construct ($player_node) {
    parent::__construct ();

    module_load_include ('inc', 'hockey_boss');
    module_load_include ('inc', 'hockey_boss_stat');
    module_load_include ('inc', 'stat');

    $this->player_node = $player_node;

    $query = stat_node_query_create (hockey_boss_stat_PFS_TYPE);
    $query = stat_node_query_add_subject_filter ($query, $player_node->nid);
    $stat_nodes = stat_node_query_execute ($query);

    $current_year = date ('Y');
    $start_year = $current_year - 2;
    foreach ($stat_nodes as $stat_node) {
      $stat_year = stat_node_get_year ($stat_node);
      if ($stat_year < $start_year) {
        $start_year = $stat_year;
      }
    }
    for ($year = $start_year; $year <= $current_year; $year ++) {
      $this->row_names [] = $year;
    }

    $this->columns = $this->create_columns (array_reverse (array ('Year', 'Circles w/o puck', 'Circles w/puck', 'Russian Circles w/o puck', 'Russian Circles w/puck', 'Sprint', 'Suicides')));

    $editable = $this->editable ();
    $this->submittable       = $editable;
    $this->sortable          = !$editable;
    $this->sort_column_name  = 'Year';
    $this->label_column_name = 'Year';
  }

  /// Returns true iff the current user has permission to edit this table.
  public function editable () {
    module_load_include ('inc', 'hockey_boss');

    global $user;
    $player_uid = hockey_boss_get_player_user_uid ($this->player_node);
    $team_node = hockey_boss_get_player_team_node ($this->player_node);
    $coach_node = is_null ($team_node) ? null : hockey_boss_get_team_coach_node ($team_node);
    $coach_uid = hockey_boss_get_coach_user_uid ($coach_node);

    return user_access ('hockey_boss_stat_edit_all_pfs') ||
           ($user->uid == $player_uid && user_access ('hockey_boss_stat_edit_own_pfs')) ||
           ($user->uid == $coach_uid  && user_access ('hockey_boss_stat_edit_team_pfs'));
  }

  /// Returns the default field values.
  public function get_default_field_value ($column_name, $row_name) {
    module_load_include ('inc', 'hockey_boss');
    module_load_include ('inc', 'hockey_boss_stat');
    module_load_include ('inc', 'stat');

    $field_width = 3;

    $stat_name = $column_name;
    $year = $row_name;

    $query = stat_node_query_create (hockey_boss_stat_PFS_TYPE);
    $query = stat_node_query_add_year_filter ($query, $year);
    $query = stat_node_query_add_title_filter ($query, $stat_name);
    $query = stat_node_query_add_subject_filter ($query, $this->player_node->nid);
    $stat_nodes = stat_node_query_execute ($query);
    $stat_node = empty ($stat_nodes) ? null : $stat_nodes [0];

    if (is_null ($stat_node)) {
      $stat_date = "$year-01-01 12:00:00";
      $stat_type = hockey_boss_stat_pfs_type ();
      $stat_node = stat_node_create ($stat_type, $stat_name, '0', '0', $stat_date, $this->player_node->nid);
      node_save ($stat_node);
    }

    $stat_value = stat_node_get_value ($stat_node);
    if ($this->editable ()) {
      return array (
        'id'            => null,
        'value'         => $stat_value,
        'display_value' => array (
          '#type' => 'container',
          "stat_value_$stat_node->nid" => array (
            '#type'               => 'textfield',
            '#default_value'      => $stat_value,
            '#size'               => $field_width
      )));
    } else {
      return array (
        'id'            => null,
        'value'         => $stat_value,
        'display_value' => $stat_value
      );
    } 
  }
}

/// Represents player game stat tables.
class PGSTable extends SimpleTable {
  /**
    The node of the player whose stats are
    displayed in this table.
  */
  public $player_node = null;

  /// Returns a player game stat table.
  public function __construct ($player_node, $year) {
    parent::__construct ();

    module_load_include ('inc', 'hockey_boss');
    module_load_include ('inc', 'hockey_boss_stat');
    module_load_include ('inc', 'stat');

    $this->player_node = $player_node;

    // I. set row names.
    $player_team_nid = hockey_boss_get_player_team_nid ($player_node);
    $game_nodes = hockey_boss_get_team_games ($player_team_nid);
    foreach ($game_nodes as $game_node) {
      $game_year = hockey_boss_get_game_year ($game_node);
      if ($game_year == $year) {
        $this->row_names [] = $game_node->nid;
      }
    }

    // II. set columns.
    $this->columns = $this->create_columns (array ('+/-', 'Face-Offs', 'Penalties', 'SAOG', 'Shots Against', 'SOG', 'Shots', 'Assists', 'Goals', 'Match', 'Date'));


    $editable = $this->editable ();
    $this->submittable      = $editable;
    $this->sortable         = !$editable;
    $this->sort_column_name = 'Date';
  }

  /// Returns true iff the current user has permission to edit this table.
  public function editable () {
    module_load_include ('inc', 'hockey_boss');

    global $user;
    $player_uid = hockey_boss_get_player_user_uid ($this->player_node);
    $team_node = hockey_boss_get_player_team_node ($this->player_node);
    $coach_node = is_null ($team_node) ? null : hockey_boss_get_team_coach_node ($team_node);
    $coach_uid = is_null ($coach_node) ? null : hockey_boss_get_coach_user_uid ($coach_node);

    return user_access ('hockey_boss_stat_edit_all_pps') ||
             ($user->uid == $player_uid && user_access ('hockey_boss_stat_edit_own_pps')) ||
             ($user->uid == $coach_uid  && user_access ('hockey_boss_stat_edit_own_team_pps'));
  }

  /// Returns the default field values.
  public function get_default_field_value ($column_name, $row_name) {
    module_load_include ('inc', 'stat');
    module_load_include ('inc', 'hockey_boss');
    module_load_include ('inc', 'hockey_boss_stat');

    $field_width = 5;

    $field_stat_name = $column_name;
    $field_stat_game_nid  = $row_name;
    $game_node = node_load ($field_stat_game_nid);
    if (!$game_node) {
      return array (
        'id'            => null,
        'value'         => '',
        'display_value' => ''
      );
    }

    if ($column_name === 'Match') {
      $game_node_uri = node_uri ($game_node);
      return array (
        'id'            => null,
        'value'         => $game_node->title,
        'display_value' => l ($game_node->title, $game_node_uri ['path'])
      );
    }
    if ($column_name === 'Date') {
      $game_date = date_format (date_create (hockey_boss_get_game_date ($game_node)), 'Y-m-d');
      return array (
        'id'            => null,
        'value'         => $game_date,
        'display_value' => $game_date
      );
    }

    $field_stat = null;
    $query = stat_node_query_create (hockey_boss_stat_PGS_TYPE);
    $query = stat_node_query_add_subject_filter ($query, $this->player_node->nid);
    $stat_nodes = stat_node_query_execute ($query);
    foreach ($stat_nodes as $stat_node) {
      $stat_game_nid = hockey_boss_stat_pgs_get_game ($stat_node);
      if ($stat_node->title === $field_stat_name &&
          $stat_game_nid === $field_stat_game_nid) {
        $field_stat = $stat_node;
        break;
      }
    }

    if (is_null ($field_stat)) {
      $stat_date = hockey_boss_get_game_date ($game_node);
      $stat_type = hockey_boss_stat_pgs_type ();
      $field_stat = stat_node_create ($stat_type, $field_stat_name, '0', '0', $stat_date, $this->player_node->nid);
      hockey_boss_stat_pgs_set_game ($field_stat, $game_node->nid);
      node_save ($field_stat);
    }

    $stat_value = stat_node_get_value ($field_stat);
    if ($this->editable ()) {
      return array (
        'id'            => null,
        'value'         => $stat_value,
        'display_value' => array (
          '#type' => 'container',
          "stat_value_$field_stat->nid" => array (
            '#type'               => 'textfield',
            '#default_value'      => $stat_value,
            '#size'               => $field_width
      )));
    } else {
      return array (
        'id'            => null,
        'value'         => $stat_value,
        'display_value' => $stat_value
      );
    } 
  }

  /// The submit callback.
  public static function process_table ($form, $form_state) {
    module_load_include ('inc', 'stat');
    foreach ($form_state ['input'] as $key => $value) {
      if (strpos ($key, 'stat_value_') === 0) {
        $stat_nid = substr ($key, 11);
        $stat_node = node_load ($stat_nid);
        if (is_null ($stat_node)) {
          continue;
        }
        stat_set_value ($stat_node, $value);
        node_save ($stat_node);
      }
    }
    drupal_set_message ('Stats updated.');
  }
}

/// Represents the team roster tables.
class TRTable extends SimpleTable {
  /**
    The team profile node of the team whose players
    are displayed in this table.
  */
  public $team_node = null;
  public $year = null;

  /// Returns a team roster table.
  public function __construct ($team_node, $year) {
    parent::__construct ();

    module_load_include ('inc', 'hockey_boss');

    $this->team_node = $team_node;
    $this->year = $year;

    // set row names.
    $this->row_names = hockey_boss_get_team_player_nids ($team_node);

    // set columns.
    $this->columns = $this->create_columns (array ('+/-', 'Face-Offs', 'Penalties', 'SAOG', 'Shots Against', 'SOG', 'Shots', 'Assists', 'Goals', 'Position', 'Name'));

    $this->sortable = true;
    $this->sort_column_name = 'Name';
  }

  /// Returns the default field values.
  public function get_default_field_value ($column_name, $row_name) {
    module_load_include ('inc', 'hockey_boss_stat');
    module_load_include ('inc', 'stat');

    $field_width = 3;

    $player_nid = $row_name;
    $player_node = node_load ($player_nid);
    if (!$player_node) {
      return array (
        'id'            => null,
        'value'         => '',
        'display_value' => ''
      );
    }

    if ($column_name === 'Name') {
      $player_node_uri = node_uri ($player_node);
      return array (
        'id'            => null,
        'value'         => $player_node->title,
        'display_value' => l ($player_node->title, $player_node_uri ['path'])
      );
    } 
    if ($column_name === 'Position') {
      $position_term = hockey_boss_get_player_position_term ($player_node);
      $position_name = is_null ($position_term) ? '' : $position_term->name;
      return array (
        'id'            => null,
        'value'         => $position_name,
        'display_value' => $position_name
      );
    }

    $stat_name = $column_name;
    $pps_query = stat_node_query_create (hockey_boss_stat_PPS_TYPE);
    $pps_query = stat_node_query_add_year_filter ($pps_query, $this->year);
    $pps_query = stat_node_query_add_subject_filter ($pps_query, $player_node->nid);
    $pps_query = stat_node_query_add_title_filter ($pps_query, $stat_name);
    $stat_nodes = stat_node_query_execute ($pps_query);
    $stat = empty ($stat_nodes) ? null : stat_from_stat_node ($stat_nodes [0]);

    if (is_null ($stat)) {
      $stat = stat_create ($stat_name, hockey_boss_stat_PPS_TYPE, 0, '', "$this->year-01-01 00:00:00", $player_nid);
      $pgs_query = stat_node_query_create (hockey_boss_stat_PGS_TYPE);
      $pgs_query = stat_node_query_add_year_filter ($pgs_query, $this->year);
      $pgs_query = stat_node_query_add_title_filter ($pgs_query, $stat_name);
      $pgs_query = stat_node_query_add_subject_filter ($pgs_query, $player_nid);
      $pgs_nodes = stat_node_query_execute ($pgs_query);

      foreach ($pgs_nodes as $pgs_node) {
        $stat ['value'] += stat_node_get_value ($pgs_node);
      }
    }

    return array (
      'id'            => null,
      'value'         => $stat ['value'],
      'display_value' => $stat ['value']
    );
  }
}

/// Represents team game stat tables.
class TGSTable extends SimpleTable {
  /// The team profile node id.
  public $team_nid = null;

  /// Returns a team game stat table.
  public function __construct ($team_node, $year) {
    $this->team_nid = $team_node->nid;

    // I. set the row names.
    $game_nodes = hockey_boss_get_team_games ($team_node->nid);
    foreach ($game_nodes as $game_node) {
      $game_year = hockey_boss_get_game_year ($game_node);
      if ($game_year == $year) {
        $this->row_names [] = $game_node->nid;
      }
    }
 
    // II. set the column names.
    $this->columns = $this->create_columns (
      array_reverse (array ('Date', 'Match', 'Home Team', 'Home Team Score', 'Away Team', 'Away Team Score')));

    $this->sortable = true;
    $this->sort_column_name = 'Date';
  }

  /// Returns the default field values.
  public function get_default_field_value ($column_name, $row_name) {
    module_load_include ('inc', 'hockey_boss');

    $game_nid = $row_name;
    $game_node = node_load ($game_nid);

    if ($column_name == 'Date') {
      $game_date_string = hockey_boss_get_game_date ($game_node);
      $game_date = new DateTime ($game_date_string);
      $game_date_string = date_format ($game_date, 'Y-m-d');
      return array (
        'id'            => null,
        'value'         => $game_date_string,
        'display_value' => $game_date_string
      );
    }

    if ($column_name == 'Match') {
      $game_node_uri = node_uri ($game_node);
      return array (
        'id'            => null,
        'value'         => $game_node->title,
        'display_value' => l ($game_node->title, $game_node_uri ['path'])
      );
    }

    $game_team_nids = hockey_boss_get_game_team_nids ($game_node);

    $home_team_nid = $game_team_nids [0];
    $away_team_nid = $game_team_nids [1];
    if ($column_name == 'Home Team') {
      $game_team_node = node_load ($home_team_nid);
      $game_team_node_uri = node_uri ($game_team_node);
      return array (
        'id'            => null,
        'value'         => $game_team_node->title,
        'display_value' => l ($game_team_node->title, $game_team_node_uri ['path'])
      );
    }

    if ($column_name == 'Away Team') {
      $game_team_node = node_load ($away_team_nid);
      $game_team_node_uri = node_uri ($game_team_node);
      return array (
        'id'            => null,
        'value'         => $game_team_node->title,
        'display_value' => l ($game_team_node->title, $game_team_node_uri ['path'])
      );
    }

    $game_team_relations = hockey_boss_get_game_team_relations ($game_node->nid);
    if ($column_name == 'Home Team Score') {
      foreach ($game_team_relations as $game_team_relation) {
        if ($game_team_relation->endpoints ['und'][1]['entity_id'] == $home_team_nid) {
          return array (
            'id'            => null,
            'value'         => $game_team_relation->{$game_team_relation->relation_type . '_score'}['und'][0]['value'],
            'display_value' => $game_team_relation->{$game_team_relation->relation_type . '_score'}['und'][0]['value']
          );
        }
      }
    }
    if ($column_name == 'Away Team Score') {
      foreach ($game_team_relations as $game_team_relation) {
        if ($game_team_relation->endpoints ['und'][1]['entity_id'] == $away_team_nid) {
          return array (
            'id'            => null,
            'value'         => $game_team_relation->{$game_team_relation->relation_type . '_score'}['und'][0]['value'],
            'display_value' => $game_team_relation->{$game_team_relation->relation_type . '_score'}['und'][0]['value']
          );
        }
      }
    }
  }
}

/// Represents the league teams tables.
class LTTable extends SimpleTable {

  /// The league profile node id.
  public $league_nid = null;

  /// Returns a league team table.
  public function __construct ($league_node) {
    parent::__construct ();

    $this->columns = $this->create_columns (
      array ('+/-', 'Face-Offs', 'Penalties', 'SAOG', 'Shots Against', 'SOG', 'Shots', 'Assists', 'Goals', 'Name'));

    $this->row_names = simple_field_get_relation_source_entity_ids ($league_node, hockey_boss_LEAGUE_PROFILE_TYPE . '_teams');

    $this->sortable = true;
    $this->sort_column_name = 'Name';
  }

  /// Returns the default field values.
  public function get_default_field_value ($column_name, $row_name) {
    module_load_include ('inc', 'stat');
    module_load_include ('inc', 'hockey_boss_stat');

    $team_nid = $row_name;
    $team_node = node_load ($team_nid);
    if (!$team_node) {
      return array (
        'id'            => null,
        'value'         => '',
        'display_value' => ''
      );
    }

    if ($column_name == 'Name') {
      $team_node_uri = node_uri ($team_node);
      return array (
        'id'            => null,
        'value'         => $team_node->title,
        'display_value' => l ($team_node->title, $team_node_uri ['path'])
      );
    }

    $stat_name = $column_name;
    $stat_value = 0;
    $current_year = date ('Y');
    $team_player_nids = simple_field_get_relation_source_entity_ids ($team_node, hockey_boss_TEAM_PROFILE_TYPE . '_players');
    foreach ($team_player_nids as $team_player_nid) {
      $query = stat_node_query_create (hockey_boss_stat_PPS_TYPE);
      $query = stat_node_query_add_year_filter ($query, $current_year);
      $query = stat_node_query_add_title_filter ($query, $stat_name);
      $query = stat_node_query_add_subject_filter ($query, $team_player_nid);
      $player_stat_nodes = stat_node_query_execute ($query);

      if (!empty ($player_stat_nodes)) {
        $player_stat_node = $player_stat_nodes [0];
        $stat_value += stat_node_get_value ($player_stat_node);
      }
    }

    return array (
      'id'            => null,
      'value'         => $stat_value,
      'display_value' => $stat_value
    );
  }
}

/// Represents the player comparison tables.
class PCTable extends SimpleTable {

  /// Returns a player comparison table.
  public function __construct ($player_nids) {
    parent::__construct ();

    // set row names.
    foreach ((array) $player_nids as $player_nid) {
      $player = node_load ($player_nid);
      if ($player) {
        $this->row_names [] = $player_nid;
      }
    }

    // set columns.
    $this->columns = $this->create_columns (
      array ('+/-', 'Face-Offs', 'Penalties', 'SAOG', 'Shots Against', 'SOG', 'Shots', 'Assists', 'Goals', 'Position', 'Name'));

    $this->sortable = true;
    $this->sort_column_name = 'Name';
  }

  /// Returns the default field values.
  public function get_default_field_value ($column_name, $row_name) {
    module_load_include ('inc', 'hockey_boss');
    module_load_include ('inc', 'hockey_boss_stat');
    module_load_include ('inc', 'stat');

    $player_nid = $row_name;
    $player = node_load ($player_nid);
    if (!$player) {
      return array (
        'id'            => null,
        'value'         => '',
        'display_value' => ''
      );
    }

    if ($column_name === 'Name') {
      $player_node_uri = node_uri ($player);
      return array (
        'id'            => null,
        'value'         => $player->title,
        'display_value' => l ($player->title, $player_node_uri ['path'])
      );
    }  
    if ($column_name === 'Position') {
      $position_term = taxonomy_term_load (hockey_boss_get_player_position_tid ($player));
      return array (
        'id'            => null,
        'value'         => $position_term->name,
        'display_value' => $position_term->name
      );
    }

    $stat_name = $column_name;
    $current_year = date ('Y');

    $query = stat_node_query_create (hockey_boss_stat_PPS_TYPE);
    $query = stat_node_query_add_year_filter ($query, $current_year);
    $query = stat_node_query_add_title_filter ($query, $stat_name);
    $query = stat_node_query_add_subject_filter ($query, $player->nid);
    $stat_nodes = stat_node_query_execute ($query);
    $stat_node = empty ($stat_nodes) ? null : $stat_nodes [0];

    if (is_null ($stat_node)) {
      $stat_date  = simple_field_create_date_field_value ($current_year, 0, 0);
      $stat_date  = $stat_date ['value'];
      $stat_type  = hockey_boss_stat_pps_type ();
      $stat_node = stat_node_create ($stat_type, $stat_name, '', '', $stat_date, $player->nid);
    }

    $stat_value = stat_node_get_value ($stat);
    $field_width = 3;
    return array (
      'id'            => null,
      'value'         => $stat_value,
      'display_value' => $stat_value
    );
  }
}
