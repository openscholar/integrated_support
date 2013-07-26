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
      'examples' => array(
        'drush @site gh2gs --labels=feature_request --style=idea --remove-labels=TRUE',    
      ),        
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
  $remove_labels = drush_get_option('remove-labels', '0');
  
  if (empty($labels)) {
    drush_set_error('You need to specify some labels.');
    return;
  } 
  
  $github = github_integration_client();
  $getsatisfaction = getsatisfaction_integration_client();
  $params = array(
    'labels' => $labels,    
  );
  $response = $github->api('issue')->all(variable_get('github_repo_owner'), variable_get('github_repo_repository'), $labels);
  
  print_r($response);
  print_r($params);
}