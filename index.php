<?php


/**
 * @file index.php
 * 
 * Desk.com <-> Github <-> GetSatisfaction   Integration 
 */

foreach (array('vendor/autoload.php', 'desk.php', 'github.php') as $file) {
  require_once($file);
}

error_reporting(E_ALL ^ E_NOTICE);

/**
 * $pages array maps path to function 
 */
$pages = array(
  'desk/create_github_issue' => 'desk_create_github_issue',
  'desk/update_test_issue' => 'desk_update_test_issue', 
  'github/close_issue' => 'github_close_issue',
  'github/close_milestone' => 'github_close_milestone',
  'app/config' => 'app_config_page',
  'app/config/desk' => 'app_config_desk_page',
  'app/config/github' => 'app_config_github',
);

/**
 * @function main
 *
 * Shows a page
 */
function main($pages) {
  $func = (isset($_GET['page'], $pages[$_GET['page']])) ? $pages[$_GET['page']] : 'status_page';
  $content = (isset($func) && function_exists($func)) ? $func() : implode("<br />\n", array_keys($pages));
  print $content;
}

main($pages);

function status_page() {
  return github_status() . "<br>\n<br>\n" . desk_status();
}


/**
 * @function json_post
 *
 * Get json data from a post var
 */
function json_post($post_var) {
  $data = (isset($_POST[$post_var])) ? $_POST[$post_var] : NULL;
  return ($data) ? json_decode($data) : NULL;
}

/**
 * @function conf
 *
 * Gets env vars and overrides for this app, returns array.
 */
function conf() {
  static $conf;

  if (empty($conf)) {
    $env_vars = array(
      'github_auth_token',
      'github_repo_owner' ,
      'github_repo_repository',
      'desk_token',
      'desk_token_secret',
      'desk_consumer_key',
      'desk_consumer_secret',
      'desk_url',
    );
    
    //read in env vars from overrides file.  key=value
    $overrides = __DIR__ . '/env_overrides.inc';
    if (is_readable($overrides) && $file = file($overrides)) {
      array_walk($file, create_function('$env', 'putenv(trim($env));'));
    }

    foreach ($env_vars as $var) {
      $conf[$var] = getenv($var);
    }

    $conf['user_map'] = array('Jon Sagotsky' => 'sagotsky'); //desk user.name -> github username mapping.  make this an env var later.
  }

  return $conf;
}


//show conf vars
function app_config_page() {
  $conf = conf();


  //env vars
  echo 'Please configure your env vars at appfog.<br />';
  $unconfigured = array_diff(array_keys($conf), array_keys(array_filter($conf)));
  if ($unconfigured) {
    echo 'The following env vars are not configured: ' . implode(', ', $unconfigured);
  }

  //liquid template vars
  require_once('liquid.inc');
  echo 'Complete list of liquid template vars available: <ul>' . implode('<li>', desk_liquid_vars()) . '</ul>';
}

function app_config_desk_page() {
  //liquid template vars
  require_once('liquid.inc');
  echo "<b>Use the following json as the desk.com app post payload</b><br /><br /\n\n";

  $vars = desk_liquid_template();
  //this approach is wrong.  some items need to be iterated over...
  echo '<code>' . $vars . '</code>';
}

//add our hooks to github
function app_config_github() {
  $gh = github_get_client();
  $conf = conf();
  $name = 'web';
  $target_url = 'http://requestb.in/wmmxenwm'; 
  $user = $conf['github_repo_owner'];
  $repo = $conf['github_repo_repository'];

  $hooks = $gh->api('repo')->hooks()->all($user, $repo);

  //delete old instance of hook.  
  foreach ($hooks as $hook) {
    if (($hook['name'] == $name) && ($hook['config']['url'] == $target_url)) {
      $gh->api('repo')->hooks()->remove($user, $repo, $hook['id']);
    }
  }

  //creates new one
  $hook_conf = array(
    'name' => $name,
    'config' => array(
      'url' => $target_url,
      'content_type' => 'json',
      'secret' => 'some secret text', 
    ),
    'events' => array('issues', 'issue_comment', 'status'),
  );
  $new = $gh->api('repo')->hooks()->create($user, $repo, $hook_conf);

  echo 'created hook:<br /><br />';
  print_r($new);

}




// function desk_http($method, $url, $parameters = array()) {
//   $conf = conf(); 
  

//   /* tapir */
//   require_once('lib/tapir/tapir.php');
//   $desk = new Tapir('desk');
//   $desk->setParameters(array('subdomain' => 'jsagotsky'));
//   $desk->useOAuth($conf['desk_consumer_key'], $conf['desk_consumer_secret'], $conf['desk_token'], $conf['desk_token_secret']);
//   $api = $desk->api('case');
//   $result = $api->call('list');
    
//   return $result;
  
// }



?>
