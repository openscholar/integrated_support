<?php
/**
 * @file
 * getsatisfaction_integration.views_default.inc
 */

/**
 * Implements hook_views_default_views().
 */
function getsatisfaction_integration_views_default_views() {
  $export = array();

  $view = new view();
  $view->name = 'getsatisfaction_integration_road_map';
  $view->description = '';
  $view->tag = 'default';
  $view->base_table = 'getsatisfaction_service_map';
  $view->human_name = 'Road Map';
  $view->core = 7;
  $view->api_version = '3.0';
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */

  /* Display: Master */
  $handler = $view->new_display('default', 'Master', 'default');
  $handler->display->display_options['title'] = 'Road Map';
  $handler->display->display_options['use_more_always'] = FALSE;
  $handler->display->display_options['access']['type'] = 'none';
  $handler->display->display_options['cache']['type'] = 'none';
  $handler->display->display_options['query']['type'] = 'views_query';
  $handler->display->display_options['exposed_form']['type'] = 'basic';
  $handler->display->display_options['pager']['type'] = 'full';
  $handler->display->display_options['pager']['options']['items_per_page'] = '10';
  $handler->display->display_options['style_plugin'] = 'table';
  /* Relationship: GH: Relate to Github */
  $handler->display->display_options['relationships']['id']['id'] = 'id';
  $handler->display->display_options['relationships']['id']['table'] = 'github_issues';
  $handler->display->display_options['relationships']['id']['field'] = 'id';
  $handler->display->display_options['relationships']['id']['required'] = TRUE;
  /* Relationship: GS: Relate to GetSatisfaction */
  $handler->display->display_options['relationships']['id_1']['id'] = 'id_1';
  $handler->display->display_options['relationships']['id_1']['table'] = 'getsatisfaction_topics';
  $handler->display->display_options['relationships']['id_1']['field'] = 'id';
  $handler->display->display_options['relationships']['id_1']['required'] = TRUE;
  /* Field: GS Service Map: ID */
  $handler->display->display_options['fields']['id']['id'] = 'id';
  $handler->display->display_options['fields']['id']['table'] = 'getsatisfaction_service_map';
  $handler->display->display_options['fields']['id']['field'] = 'id';
  $handler->display->display_options['fields']['id']['label'] = '';
  $handler->display->display_options['fields']['id']['exclude'] = TRUE;
  $handler->display->display_options['fields']['id']['element_label_colon'] = FALSE;
  /* Field: GS: Topic Title */
  $handler->display->display_options['fields']['title']['id'] = 'title';
  $handler->display->display_options['fields']['title']['table'] = 'getsatisfaction_topics';
  $handler->display->display_options['fields']['title']['field'] = 'title';
  $handler->display->display_options['fields']['title']['label'] = 'Title';
  $handler->display->display_options['fields']['title']['element_label_colon'] = FALSE;
  /* Field: GH: Updated */
  $handler->display->display_options['fields']['updated']['id'] = 'updated';
  $handler->display->display_options['fields']['updated']['table'] = 'github_issues';
  $handler->display->display_options['fields']['updated']['field'] = 'updated';
  $handler->display->display_options['fields']['updated']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['updated']['date_format'] = 'os_time';
  /* Field: GS: milestone */
  $handler->display->display_options['fields']['milestone']['id'] = 'milestone';
  $handler->display->display_options['fields']['milestone']['table'] = 'getsatisfaction_topics';
  $handler->display->display_options['fields']['milestone']['field'] = 'milestone';
  $handler->display->display_options['fields']['milestone']['label'] = 'Scheduled Milestone';
  $handler->display->display_options['fields']['milestone']['element_label_colon'] = FALSE;
  /* Field: GS: Status */
  $handler->display->display_options['fields']['status']['id'] = 'status';
  $handler->display->display_options['fields']['status']['table'] = 'getsatisfaction_topics';
  $handler->display->display_options['fields']['status']['field'] = 'status';
  $handler->display->display_options['fields']['status']['element_label_colon'] = FALSE;
  /* Field: GS: body */
  $handler->display->display_options['fields']['body']['id'] = 'body';
  $handler->display->display_options['fields']['body']['table'] = 'getsatisfaction_topics';
  $handler->display->display_options['fields']['body']['field'] = 'body';
  $handler->display->display_options['fields']['body']['label'] = '';
  $handler->display->display_options['fields']['body']['element_class'] = 'issue-body';
  $handler->display->display_options['fields']['body']['element_label_colon'] = FALSE;
  /* Filter criterion: GS: milestone */
  $handler->display->display_options['filters']['milestone']['id'] = 'milestone';
  $handler->display->display_options['filters']['milestone']['table'] = 'getsatisfaction_topics';
  $handler->display->display_options['filters']['milestone']['field'] = 'milestone';
  $handler->display->display_options['filters']['milestone']['exposed'] = TRUE;
  $handler->display->display_options['filters']['milestone']['expose']['operator_id'] = 'milestone_op';
  $handler->display->display_options['filters']['milestone']['expose']['label'] = 'milestone';
  $handler->display->display_options['filters']['milestone']['expose']['operator'] = 'milestone_op';
  $handler->display->display_options['filters']['milestone']['expose']['identifier'] = 'milestone';
  $handler->display->display_options['filters']['milestone']['is_grouped'] = TRUE;
  $handler->display->display_options['filters']['milestone']['group_info']['label'] = 'Milestone';
  $handler->display->display_options['filters']['milestone']['group_info']['identifier'] = 'milestone';
  $handler->display->display_options['filters']['milestone']['group_info']['group_items'] = array(
    1 => array(
      'title' => '',
      'operator' => '=',
      'value' => '',
    ),
    2 => array(
      'title' => '',
      'operator' => '=',
      'value' => '',
    ),
    3 => array(
      'title' => '',
      'operator' => '=',
      'value' => '',
    ),
  );

  /* Display: Page */
  $handler = $view->new_display('page', 'Page', 'page');
  $handler->display->display_options['defaults']['hide_admin_links'] = FALSE;
  $handler->display->display_options['path'] = 'roadmap';
  $export['getsatisfaction_integration_road_map'] = $view;

  return $export;
}
