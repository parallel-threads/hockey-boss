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
  public function __construct ($player_nid) {
    parent::__construct ();
    
    $this->player_nid = $player_nid;

    // set row names.
    $start_year = '2013';
    $current_year = date ('Y');
    for ($year = $start_year; $year <= $current_year; $year ++) {
      $this->row_names [] = $year;
    }

    // set columns.
    $this->columns = $this->create_columns (
      array ('+/-', 'Face-Offs', 'Penalties', 'Shots Against On Goal', 'Shots Against', 'Shots On Goal', 'Shots', 'Assists', 'Goals', 'Year'));

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
    $player = node_load ($this->player_nid);

    $player_uid = hockey_boss_get_player_user_uid ($player);

    $coach_uid = hockey_boss_get_coach_user_uid (
                   node_load (hockey_boss_get_team_coach_nid (
                     node_load (hockey_boss_get_player_team_nid ($player)))));
/*
    watchdog (
      'hockey_boss_table',
      '[PPSTable::editable] user id: @uid player user id: @player_uid coach user id: @coach_uid',
      array (
        '@uid' => $user->uid,
        '@player_uid' => $player_uid,
        '@coach_uid'   => $coach_uid
    ));
*/
    return user_access ('hockey_boss_stat_edit_all_pps') ||
           ($user->uid == $player_uid && user_access ('hockey_boss_stat_edit_own_pps')) ||
           ($user->uid == $coach_uid  && user_access ('hockey_boss_stat_edit_team_pps'));
  }

  /// Returns the default field values.
  public function get_default_field_value ($column_name, $row_name) {
    module_load_include ('inc', 'stat');
    module_load_include ('inc', 'hockey_boss_stat');

    $field_width = 5;

    $field_stat_name = $column_name;
    $field_stat_year = $row_name;

    $pps_query = stat_node_query_create (hockey_boss_stat_PPS_TYPE);
    $pps_query = stat_node_query_add_year_filter ($pps_query, $field_stat_year);
    $pps_query = stat_node_query_add_title_filter ($pps_query, $field_stat_name);
    $pps_query = stat_node_query_add_subject_filter ($pps_query, $this->player_nid);
    $stat_nodes = stat_node_query_execute ($pps_query);
    $field_stat = empty ($stat_nodes) ? null : stat_from_stat_node ($stat_nodes [0]);

    if (is_null ($field_stat)) {
      $field_stat = stat_create ($field_stat_name, hockey_boss_stat_PPS_TYPE, 0, '', "$field_stat_year-01-01 00:00:00", $this->player_nid);

      $pgs_query = stat_node_query_create (hockey_boss_stat_PGS_TYPE);
      $pgs_query = stat_node_query_add_year_filter ($pgs_query, $field_stat_year);
      $pgs_query = stat_node_query_add_title_filter ($pgs_query, $field_stat_name);
      $pgs_query = stat_node_query_add_subject_filter ($pgs_query, $this->player_nid);
      $pgs_nodes = stat_node_query_execute ($pgs_query);

      foreach ($pgs_nodes as $pgs_node) {
        $field_stat ['value'] += stat_node_get_value ($pgs_node);
      }
    }

    if ($this->editable () && isset ($field_stat ['nid'])) {
      return array (
        'id'            => null,
        'value'         => $field_stat ['value'],
        'display_value' => array (
          '#type' => 'container',
          'stat_value_' . $field_stat ['nid'] => array (
            '#type'               => 'textfield',
            '#default_value'      => $field_stat ['value'],
            '#size'               => $field_width
      )));
    } else {
      return array (
        'id'            => null,
        'value'         => $field_stat ['value'],
        'display_value' => $field_stat ['value']
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

/// Represents player game stat tables.
class PGSTable extends SimpleTable {
  /**
    The node id of the player whose stats are
    displayed in this table.
  */
  public $player = null;

  /// Returns a player game stat table.
  public function __construct ($player) {
    parent::__construct ();

    module_load_include ('inc', 'hockey_boss');
    module_load_include ('inc', 'hockey_boss_stat');
    module_load_include ('inc', 'stat');

    $this->player = $player;

    // I. set row names.
    $player_team_nid = hockey_boss_get_player_team_nid ($player);
    $games = hockey_boss_get_game_nodes ();
    $game_nids = array ();
    foreach (hockey_boss_get_game_nodes () as $game) {
      $game_team_nids = hockey_boss_get_game_team_nids ($game);
      if ($player_team_nid == $game_team_nids [0] ||
          $player_team_nid == $game_team_nids [1]) {
        $game_nids [] = $game->nid;
      }
    }

    $this->row_names = $game_nids;

    // II. set columns.
    $column_index = 0;
    $column_names = array ('+/-', 'Face-Offs', 'Penalties', 'Shots Against On Goal', 'Shots Against', 'Shots On Goal', 'Shots', 'Assists', 'Goals', 'Date', 'Game');
    foreach ($column_names as $column_name) {
      $this->columns [] = array (
        'name'   => $column_name,
        'label'  => $column_name,
        'weight' => $column_index
      );
     $column_index ++;
    }

    $editable = $this->editable ();
    $this->submittable      = $editable;
    $this->sortable         = !$editable;
    $this->sort_column_name = 'Date';
  }

  /// Returns true iff the current user has permission to edit this table.
  public static function editable () {
    return false;
  }

  /// Returns the default field values.
  public function get_default_field_value ($column_name, $row_name) {
    module_load_include ('inc', 'stat');
    module_load_include ('inc', 'hockey_boss');
    module_load_include ('inc', 'hockey_boss_stat');

    $player = $this->player;

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

    if ($column_name === 'Game') {
      return array (
        'id'            => null,
        'value'         => $game_node->title,
        'display_value' => $game_node->title
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
    $query = stat_node_query_add_subject_filter ($query, $player->nid);
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
      $field_stat = stat_node_create ($stat_type, $field_stat_name, '0', '0', $stat_date, $player->nid);
      hockey_boss_stat_pgs_set_game ($field_stat, $game_node->nid);
      node_save ($field_stat);
    }

    $stat_value = stat_get_value ($field_stat);
    if ($this->editable ()) {
      return array (
        'id'            => null,
        'value'         => $stat_value,
        'display_value' => array (
          '#type' => 'container',
          "stat_value_$stat->nid" => array (
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
  public $team = null;

  /// Returns a team roster table.
  public function __construct ($team) {
    parent::__construct ();

    module_load_include ('inc', 'hockey_boss');

    $this->team = $team;

    // set row names.
    $this->row_names = hockey_boss_get_team_player_nids ($team);

    // set columns.
    $this->columns = $this->create_columns (
      array ('+/-', 'Face-Offs', 'Penalties', 'Shots Against On Goal', 'Shots Against', 'Shots On Goal', 'Shots', 'Assists', 'Goals', 'Position', 'Name'));

    $this->sortable = true;
    $this->sort_column_name = 'Name';
  }

  /// Returns the default field values.
  public function get_default_field_value ($column_name, $row_name) {
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
      $player_profile_link = l ($player->title, $player_node_uri ['path']);
      return array (
        'id'            => null,
        'value'         => $player->title,
        'display_value' => $player_profile_link
      );
    } 
    if ($column_name === 'Position') {
      // To Be Implemented
      return array (
        'id'            => null,
        'value'         => '',
        'display_value' => ''
      );
    }

    $stat_name = $column_name;
    $current_year = date ('Y');
    $query = stat_node_query_create (hockey_boss_stat_PPS_TYPE);
    $query = stat_node_query_add_year_filter ($query, $current_year);
    $query = stat_node_query_add_subject_filter ($query, $player->nid);
    $query = stat_node_query_add_title_filter ($query, $stat_name);
    $stat_nodes = stat_node_query_execute ($query);
    $stat_node = empty ($stat_nodes) ? null : $stat_nodes [0];

    if (is_null ($stat_node)) {
      $stat_date  = simple_field_create_date_field_value ($current_year, 0, 0);
      $stat_date  = $stat_date ['value'];
      $stat_type  = hockey_boss_stat_pps_type ();
      $stat_node = stat_node_create ($stat_type, $stat_name, '0', '0', $stat_date, $player->nid);
    }

    $stat_value = stat_node_get_value ($stat_node);
    $field_width = 3;
    return array (
      'id'            => null,
      'value'         => $stat_value,
      'display_value' => $stat_value
    );
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
      array ('+/-', 'Face-Offs', 'Penalties', 'Shots Against On Goal', 'Shots Against', 'Shots On Goal', 'Shots', 'Assists', 'Goals', 'Name'));

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
      array ('+/-', 'Face-Offs', 'Penalties', 'Shots Against On Goal', 'Shots Against', 'Shots On Goal', 'Shots', 'Assists', 'Goals', 'Position', 'Name'));

    $this->sortable = true;
    $this->sort_column_name = 'Name';
  }

  /// Returns the default field values.
  public function get_default_field_value ($column_name, $row_name) {
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
      // To Be Implemented
      return array (
        'id'            => null,
        'value'         => '',
        'display_value' => ''
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
