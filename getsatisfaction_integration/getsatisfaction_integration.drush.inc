<?php

/**
 * @file getsatisfaction_integration.drush.inc
 * 
 * Drush commands for GS integration
 */

/**
 * Impelments hook_drush_command
 */
function getsatisfaction_integration_drush_command() {
  return array(
    'github-to-getsatisfaction' => array(
      'description' => dt('Copies github tickets to getsatisfaction'),
      'options' => array(
        'labels' => dt('A comma separated list of labels to search for.  Github will return tickets that match ALL the labels.  (Required)'),
        'style' => dt('GetSatisfaction topics should use this style.  Valid options are question, problem, praise, idea, and update (default: question)'),
        'remove-labels' => dt('After creating GS topic, should the original github ticket be stripped of its labels so it won\'t be scanned again? (default: 0)')   
      ),    
      'arguments' => array(),
      'examples' => array('drush @site gh2gs --labels=feature_request --style=idea --remove-labels=TRUE',),        
      'aliases' => array('gh2gs'),  
    ),
  );
}

/**
 * Implements drush_hook_command
 */
function drush_getsatisfaction_integration_github_to_getsatisfaction() {
  //get and check options
  $labels = drush_get_option('labels', '');
  $style = drush_get_option('style','question');
  
  if (empty($labels)) {
    drush_set_error('You need to specify some labels.');
    return;
  } 
  
  if (!in_array($style, array('question', 'problem', 'praise', 'idea', 'update'))) {
    drush_set_error('Invalid style');
    return;
  }
  
  $github = github_integration_client();
  $params = array('labels' => $labels);
  
  try { 
    $response = $github->api('issue')->all(variable_get('github_repo_owner'), variable_get('github_repo_repository'), $params);
  } catch (Exception $e) {
    drush_set_error('Github is being a bitch, yo.  @code, @msg', array('@code' => $e->getCode(), '@msg' => $e->getMessage()));
    return;
  }

  array_map('_drush_getsatisfaction_integration_issue_to_topic', $response);
}

/**
 * @function _drush_getsatisfaction_integration_issue_to_topic
 * 
 * Given a github issue, create a complementary getsatisfaction ticket.
 */
function _drush_getsatisfaction_integration_issue_to_topic($issue) {
  $getsatisfaction = getsatisfaction_integration_client();
  $style = drush_get_option('style','question');
  $remove_labels = drush_get_option('remove-labels', '0');
  
  $topic = getsatisfaction_create_topic($issue['title'], $issue['body'], $style);
  if ($topic) {
    //topic created.  link it to the GH issue.
    $state = array('values' => array(
      'external_service' => 'github_integration',
      'external_id' => $issue['number'],
      'getsatisfaction_url' => $topic->at_sfn,
    ));
    
    require_once(drupal_get_path('module', 'getsatisfaction_integration') . '/getsatisfaction_integration.admin.inc');
    drupal_form_submit('getsatisfaction_integration_admin_tickets', $state);
  
    if ($remove_labels) {
      _drush_getsatisfaction_integration_issue_clear_labels($issue);
    }
  }
}

/**
 * @function _drush_getsatisfaction_integration_issue_clear_labels
 * 
 * Remove the labels that were used to find this ticket.  This way we can run command several times without getting redundant issues
 */
function _drush_getsatisfaction_integration_issue_clear_labels($issue) {
  $github = github_integration_client();
  $labels = array_map(create_function('$a', 'return $a["name"];'), $issue['labels']);
  $remove = array_filter(explode(',', drush_get_option('labels', '')));
  $replace = array_diff($labels, $remove);
    
  $github->api('issue')->labels()->replace(variable_get('github_repo_owner'), variable_get('github_repo_repository'), $issue['number'], $replace);
}