<?php 


function theme_github_integration_issue($variables) {
  drupal_add_css(drupal_get_path('module', 'github_integration') . '/github_integration_labels.css');
  
  $issue = $variables['issue'];
  $label_text = theme('github_integration_issue_labels', array('labels' => $issue['labels']));
  
  $time = ($issue['state'] != 'closed') ? $issue['created_at'] : $issue['closed_at'];
  $ago = format_interval(time() - strtotime($time), 1);
  $updated = ($issue['state'] != 'closed') ? "Reported $ago ago" : "Closed $ago ago";
  
  $link = l($issue['title'], $issue['html_url'], array('attributes' => array(
    'class' => array(
      'github-state-' . $issue['state'],
    ),    
  )));
  
  return $link . '(' . $updated . ' ' . $label_text . ')';
}

function theme_github_integration_issue_labels($variables) {
  $ret = '';
  foreach ($variables['labels'] as $label) {
    //determine brightness so we can use black or white text.
    $bg  = $label['color'];
    $lightness = 0;
    foreach (array($bg[0], $bg[2], $bg[4]) as $c) {
      if (!(is_numeric($c) && $c<8)) {
        $lightness++;
      }
    }
    
    $color = ($lightness > 1) ? 'black' : 'white';
    //$ret .= ' <span class="github-label" style="color: '.$color.'; background-color:#'.$bg.'">' . $label['name'] . '</span>';
    $ret .= theme('html_tag', (array('element' => array(
      '#tag' => 'span',
      '#value' => $label['name'],
      '#attributes' => array(
        'class' => array(
          'github-label',
        ),
        'style' => array(
          "color: $color;",
          "background-color: #$bg",
        )
    )))));
  }
  
  return $ret;
}