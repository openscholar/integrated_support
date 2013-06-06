<?php


//use Buzz\Listener\OAuthListener;

/**
 * @file index.php
 * 
 * Desk.com <-> Github <-> GetSatisfaction   Integration 
 */

require_once('vendor/autoload.php');
error_reporting(E_ALL ^ E_NOTICE);
// use Desk\Client;

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
 * @function desk_create_github_issue
 *
 * Callback function for creating a github issue from a desk.com api post
 */
function desk_create_github_issue() {
  if (!($data = json_post('data'))) {
    error_log('No json data in post.  $_POST:' . var_export($_POST, TRUE));
  }

  $conf = conf();
  $gh = github_get_client();

  //create an issue
  $issue_body = array('**From Desk.com** ' . 'http://jsagotsky.desk.com/agent/case/'.$data->case_id);

  foreach ($data->case_body as $body) {
    $issue_body[] = "```\n$body\n```\n";
  }
  //$issue_body[] = "```\n$data->case_body\n```";

  //browser stats
  $head = $sep = $tbody = array();
  foreach (array('os', 'os_version', 'browser', 'browser_version') as $field) {
    if ($data->{$field}) {
      $head[] = $field;
      $sep[] = '---';
      $tbody[] = $data->{$field};
    }
  }

  if ($head) {
    $issue_body[] = '|' . implode('|', $head) . '|';
    $issue_body[] = '|' . implode('|', $sep) . '|';
    $issue_body[] = '|' . implode('|', $tbody) . '|';
  }

  $issue = array(
    'title' => ($data->case_subject) ? $data->case_subject : 'default title',
    'assignee' => (isset($conf['user_map'][$data->case_user])) ? $conf['user_map'][$data->case_user] : NULL,
    'labels' => 'desk',
    'body' => implode("\n", $issue_body),
  );

  $i = $gh->api('issue')->create($conf['github_repo_owner'], $conf['github_repo_repository'], $issue);
  
//   $gh_properties = array(
//     'github_issue_id' => $i['number'],
//     'github_status' => $i['state'],
//   );
  
//   if ($i['milestone']) {
//     $gh_properties['github_milestone'] = $i['milestone'];
//   }
  $gh_properties = new stdClass;
  $gh_properties->github_issue_id = $i['number'];
  $gh_properties->github_status = $i['state'];
      
  if ($i['milestone']) {
    $gh_properties->github_milestone = $i['milestone'];
  }
  
  
  //take $i and send id or number back to desk.com
  $update = array('id' => $data->case_id, 'custom_fields' => $gh_properties);
  $desk = desk_get_client();
  $desk_out = $desk->api('case')->call('update', $update);
   
  error_log('Payload: ' . json_encode($update));
  error_log('Response: ' . var_export($desk_out, TRUE));
}


function desk_update_test_issue() {
  $desk = desk_get_client();
  
  $custom = array('github_issue_id'=>'0');
  
  $update = array(
     'id' => 4,
     'subject' => 'this case has been updated via the api',
     'custom_fields' => $custom, 
  );
  
  //var_export($update);
     
  
  //custom fields are part of curls data body.  is that being pushed right?
  $ret = $desk->api('case')->call('update', $update); 
  return "\n\n<br><Br>" . var_export($ret, TRUE);
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

function github_get_client($auth = TRUE) {
  static $gh_client;
  if (!empty($gh_client)) {
    return $gh_client;
  }

  $conf = conf();
  $cache = new Github\HttpClient\CachedHttpClient(array('cache_dir' => __DIR__ . '/.github-api-cache'));
  $gh_client = new Github\Client($cache);

  if ($auth) {
    try {
      $gh_client->authenticate($conf['github_auth_token'], '', Github\Client::AUTH_HTTP_PASSWORD);
    } catch (Exception $e) {
      print 'error';
      $gh_client = NULL;
    }
  }

  return $gh_client;
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

/**
 * @function github_status
 * 
 * Checks that github can auth 
 */
function github_status() {
  $gh = github_get_client();
  $me = $gh->api('current_user')->show();
  return ($me) ? 'Logged into github as: ' . $me['login'] : 'Could not auth github';
}

/**
 * @function desk_status
 * 
 * Checks that desk can auth
 */
function desk_status() {
  $desk = desk_get_client();
  $search = $desk->api('case')->call('search', array('case_id' => 4));
  return (isset($search->total_entries)) ? 'Logged into desk.com' : 'Error logging in to desk.';
//   $json = desk_http('get', 'https://jsagotsky.desk.com/api/v2/cases/search', array('case_id' => '4'));
//   return ($json && (!isset($json->message) || $json->message != 'Unauthorized') && empty($json->errors)) ? 'Logged into desk.com' : 'Error logging into desk.';
   
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

function desk_get_client() {
  static $desk_client;
  if (!empty($desk_client)) {
    return $desk_client;
  }
  
  $conf = conf();
  require_once('lib/tapir/tapir.php');

  $desk = new Tapir('desk');
  $desk->setParameters(array('subdomain' => 'jsagotsky'));
  $desk->useOAuth($conf['desk_consumer_key'], $conf['desk_consumer_secret'], $conf['desk_token'], $conf['desk_token_secret']);
 
  return $desk;
}


/**
 * @function github_hook_comments
 * 
 * Recieves github's webhook for comments being added to an issue
 */
function github_hook_comment() {
  $body = file_get_contents(STDIN);
  $json = json_decode($body);
  
  if ($json->action != 'created') {
    return;
  }
  
  $id = $json->issue['number'];
  $milestone = $json->issue['milestone']['title'];
  $state = $json->issue['state'];
  
  $desk = desk_get_client();
  //https://yoursite.desk.com/api/v2/cases/search\?subject\=please+help\&name\=jimmy\&page\=1\&per_page\=2 \

}

/**
 * @function github_hook_issue
 *
 * Recieves github's webhook for issues being created or deleted
 */
function github_hook_issue() {
  
}

?>
