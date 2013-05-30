<?php


use Buzz\Listener\OAuthListener;

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

  //print_r($gh);
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

  //take $i and send id or number back to desk.com
  print_r($i);
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
  $cache = new Github\HttpClient\CachedHttpClient(array('cache_dir' => __DIR__ . '/github-api-cache'));
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
  $conf = conf();
  require_once('lib/OAuth-PHP/OAuth.php');
  $desk_consumer = new OAuthConsumer($conf['desk_consumer_key'], $conf['desk_consumer_secret']);
  $desk_token = new OAuthToken($conf['desk_token'], $conf['desk_token_secret']);
  //print_r($desk_oauth);
  $req = new OAuthRequest('get', 'https://jsagotsky.desk.com/api/v2/cases/4');
  //print_r($req->from_consumer_and_token($consumer, $token, $http_method, $http_url));
  $from = OAuthRequest::from_consumer_and_token($desk_consumer, $desk_token, 'get', 'https://jsagotsky.desk.com/api/v2/cases/4');
  $from->sign_request(new OAuthSignatureMethod_PLAINTEXT(), $desk_consumer, $desk_token);
  
  print_r($from->to_url());
  $opts = array(
      'http'=>array(
          'method'=>"GET",
          'header'=>$from->to_header(),
          )
      );
          
          
print_r($from->to_header());
  $url = $from->to_url();
  $context = stream_context_create($opts);
//  print_r(file_get_contents($url, false, $context));
  
  $ch = curl_init();
  curl_setopt($ch, CURLOPT_URL, $url);
  $data = curl_exec($ch);
  curl_close($ch);
  
  print_r($data);
  
  
  //lets try buzz...  hard to get oauth talking.  looks like the gh version did something gh specific - basic auth with a magic token
//   $browser = new Buzz\Browser();
//   $browser->addListener(new OAuthListener('AUTH_URL_CLIENT_ID', array('client_id' => $conf['desk_token'], 'client_secret' => $conf['desk_token_secret'])));
//   $browser->get('https://jsagotsky.desk.com/api/v2/cases/4');
//   print_r($browser->getLastRequest());
  
  // https://gist.github.com/jrmey/2564090/abe665155c6fcc417b470913358f0f8c5305b4ab
//   try {
//     //Create a new Oauth request.
//     $oauth = new OAuth($conf['desk_consumer_key'], $conf['desk_consumer_secret']);
//     $oauth->enableDebug();
//     $oauth->setToken($conf['desk_token'], $conf['desk_token_secret']);
//     $query = '';
//     $my_desk_url = 'http://jsagotsky.desk.com';
//     $oauth->fetch($my_desk_url."/api/v1/cases.json".$query,array(),OAUTH_HTTP_METHOD_GET);
//     $json = json_decode($oauth->getLastResponse());
//     return var_export($json, TRUE);
//   } catch (Exception $e) {
//     print_r($e);
//     return FALSE;
//   }
  
}

// function desk_status() {

// this function uses bradfeehan's desk-php.  seems unstable so far.

//   //use Guzzle\Service\Builder;
//   $conf = conf();
//   $config = array();
//   foreach(array('token', 'token_secret', 'consumer_key', 'consumer_secret') as $value) {
//     $config[$value] = $conf['desk_' . $value];
//   }
//   $config['subdomain'] = 'jsagotsky';

//   //$desk = new Desk\Client($conf['desk_url'], $config);
// //   $factory = $desk::factory($config);
//   $desk = Desk\Client::factory($config);
//   $cmd = $desk->getCommand('ListCases', array());
//   $results = $desk->execute($cmd);
  
//   print_r($results);
  
// //   $customers = $results->getEmbedded('entries');
// //   print_r($customers);
  
// //   $result = $cmd->getResponse();
// //   print_r($result);
//   //$desk = new Factory();
//   //Guzzle\Service\Builder\ServiceBuilder(array());//::factory($config);
  
//   return 'var_export($desk, TRUE)';
// }


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
  
  //add note to desk.com case?
//   if (strpos($json->comment['body'], '@desk.com') !== FALSE) {
//     $comment = $json->comment['body'];
//   }

}

/**
 * @function github_hook_issue
 *
 * Recieves github's webhook for issues being created or deleted
 */
function github_hook_issue() {
  
}

?>
