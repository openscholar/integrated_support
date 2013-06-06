<?php


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
