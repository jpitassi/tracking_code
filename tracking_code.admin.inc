<?php

/**
 * @file
 * admin page callbacks and form handlers for the tracking code module
 */

/**
 * Page callback for tracking code admin page (admin/structure/tracking_code).
 *
 * @return array
 *   the themed output for the admin overview page
 */
function tracking_code_admin_overview() {
  drupal_add_css(drupal_get_path('module', 'tracking_code') . '/css/tracking_code.css');
  drupal_add_js(drupal_get_path('module', 'tracking_code') . '/js/tracking_code.js');
  $output = drupal_get_form('tracking_code_overview_form');

  return $output;
}

/**
 * Theme function for the tracking code admin overview table.
 *
 * @param array $variables
 *   an associative array of arguments (should only contain $form array)
 *
 * @return array
 *   the admin overview form, themed as a draggable table of tracking_code
 */
function theme_tracking_code_overview_table($variables) {
  global $base_url;
  $output = '';
  $form = $variables['form'];
  $regions = unserialize($form['regions']['#value']);

  if (empty($regions['header']) && empty($regions['page_top']) && empty($regions['page_bottom'])) {
    $link = l(t('Click here'), 'admin/structure/tracking_code/add');
    $output .= t('You have not configured any tracking code snippets. !link to add one.', array('!link' => $link));
    return $output;
  }

  // Create a table for each region.
  foreach ($regions as $region => $snippets) {
    $rows = array();
    if (empty($snippets)) {
      // Nothing to do here...
      continue;
    }

    // Sort rows by weight before creating table.
    usort($snippets, '_tracking_code_weight_sort');

    // Build a row for each snippet.
    foreach ($snippets as $id => $snippet) {
      $actions = array();
      $enabled = ($snippet->status) ? t('Disable') : t('Enable');
      $actions = array(
        l($enabled, 'admin/structure/tracking_code/', array(
          'attributes' => array('class' => 'tracking-code-disable-link', 'rel' => $snippet->tcid),
          'html' => TRUE,
          'fragment' => '',
        )),
        l(t('Configure'), 'admin/structure/tracking_code/' . $snippet->tcid . '/edit'),
        l(t('Delete'), 'admin/structure/tracking_code/' . $snippet->tcid . '/delete'),
      );

      $rows[] = array(
        'data' => array(
          array(
            'data' => check_plain($snippet->name),
            'class' => 'tracking-code-name',
          ),
          array(
            'data' => check_plain(truncate_utf8($snippet->code, 150, FALSE, '...')),
            'class' => 'tracking-code-code',
          ),
          implode(' | ', $actions),
          drupal_render($form['tracking_code'][$region]['weight_' . $snippet->tcid]),
        ),
        // Make all rows draggable.
        'class' => ($snippet->status) ? array('draggable') : array('draggable tracking-code-disabled'),
      );
    }

    switch ($region) {
      case 'header':
        $output .= '<h2>In &lt;HEAD&gt;</h2>';
        break;

      case 'page_top':
        $output .= '<h2>After &lt;BODY&gt;</h2>';
        break;

      case 'page_bottom':
        $output .= '<h2>Before &lt;/BODY&gt;</h2>';
        break;
    }

    $output .= theme('table', array(
      'header' => array(t('Name'), t('Code'), t('Actions'), t('Weight')),
      'rows' => $rows,
      'attributes' => array('id' => 'tracking-code-admin-table-' . $region),
    ));

    drupal_add_tabledrag('tracking-code-admin-table-' . $region, 'order', 'siblings', 'snippet-weight');
  }

  $output .= drupal_render_children($form);

  return $output;
}

/**
 * Form for tracking code overview table.
 *
 * @return array
 *   a renderable Form API array
 */
function tracking_code_overview_form() {
  $regions = _tracking_code_all_by_region();

  $form = array(
    '#type' => 'form',
    '#theme' => 'tracking_code_overview_table',
    '#theme_wrappers' => array('form'),
  );

  $form['regions'] = array(
    '#type' => 'value',
    '#value' => serialize($regions),
  );

  foreach ($regions as $region => $snippets) {
    foreach ($snippets as $id => $snippet) {
      // Build a form element for each weight attribute.
      $form['tracking_code'][$region]['weight_' . $id] = array(
        '#type' => 'textfield',
        '#default_value' => $snippet->weight,
        '#attributes' => array('class' => array('snippet-weight'), 'size' => 3),
      );
    }
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Form for adding codeblocks (admin/structure/tracking_code/add).
 *
 * @return array
 *   a renderable Form API array
 */
function tracking_code_add_form() {
  $form['tracking_code_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#description' => t('Enter a name to describe this tracking code snippet'),
  );

  $form['tracking_code_code'] = array(
    '#type' => 'textarea',
    '#title' => t('Code'),
    '#description' => t('Paste your tracking code here'),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save Snippet'),
  );

  return $form;
}

/**
 * Form for editing codeblocks (admin/structure/tracking_code/%/edit).
 *
 * @param (int) $delta
 *   the array index of the codeblock to edit
 *
 * @return array
 *   a renderable Form API array
 */
function tracking_code_edit_form($form, &$form_state, $delta) {
  $snippet = _tracking_code_read($delta);

  // Don't show a blank edit form.
  if (!$snippet) {
    drupal_not_found();
    exit();
  }

  $form['delta'] = array(
    '#type' => 'hidden',
    '#value' => $delta,
  );

  $form['tracking_code_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#description' => t('Enter a name to describe this code block'),
    '#default_value' => $snippet['name'],
  );

  $form['tracking_code_status'] = array(
    '#type' => 'radios',
    '#title' => t('Enable Tracking Code'),
    '#description' => t("Tracking code snippets are disabled by default, so you won't accidentally make tracking code live if you didn't intend to."),
    '#default_value' => $snippet['status'],
    '#options' => array(1 => t('Active'), 0 => t('Disabled')),
  );

  $form['tracking_code_code'] = array(
    '#type' => 'textarea',
    '#title' => t('Code'),
    '#description' => t('Enter your tracking code snippet here'),
    '#default_value' => $snippet['code'],
  );

  $form['tracking_code_options'] = array(
    '#type' => 'fieldset',
    '#title' => t('Visibility Settings'),
    '#collapsible' => TRUE,
  );

  $form['tracking_code_options']['tracking_code_region'] = array(
    '#type' => 'radios',
    '#title' => t('Render this code:'),
    '#options' => array(
      'header' => t('Inside &lt;HEAD&gt;'),
      'page_top' => t('After &lt;BODY&gt;'),
      'page_bottom' => t('Before &lt;/BODY&gt;'),
    ),
    '#default_value' => $snippet['region'],
  );

  $form['tracking_code_options']['tracking_code_visibility'] = array(
    '#type' => 'radios',
    '#title' => t('Invoke this tracking code on specific pages:'),
    '#options' => array(
      TRACKING_CODE_VISIBILITY_NOTLISTED => t('All pages except those listed'),
      TRACKING_CODE_VISIBILITY_LISTED => t('Only the listed pages'),
    ),
    '#default_value' => $snippet['visibility'],
  );

  $form['tracking_code_options']['tracking_code_pages'] = array(
    '#type' => 'textarea',
    '#title' => t('Pages'),
    '#description' => t("Specify pages by using their paths. Enter one path per line. The '*' character is a wildcard. Example paths are blog for the blog page and blog/* for every personal blog. <front> is the front page."),
    '#default_value' => $snippet['pages'],
  );

  $form['tracking_code_options']['tracking_code_content_types'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Content Types'),
    '#options' => node_type_get_names(),
    '#default_value' => unserialize($snippet['content_types']),
    '#description' => t('Show this tracking code only on pages that display content of the given type(s). If you select no types, there will be no type-specific limitation.'),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save Snippet'),
  );

  return $form;
}

/**
 * Form for deleting codeblocks (admin/structure/tracking_code/%/delete).
 *
 * @param (int) $delta
 *   the array index of the codeblock to edit
 *
 * @return array
 *   a renderable Form API array or 404
 */
function tracking_code_delete_form($form, &$form_state, $delta) {
  $snippet = _tracking_code_read($delta);

  // Don't show a blank delete form.
  if (!$snippet) {
    drupal_not_found();
    exit();
  }

  $form['delta'] = array('#type' => 'hidden', '#value' => $delta);
  $form['snippet_name'] = array('#type' => 'hidden', '#values' => $snippet['name']);

  return confirm_form($form, t('Are you sure you want to delete the tracking code snippet %name?', array('%name' => $snippet['name'])), 'admin/structure/tracking_code', '', t('Delete'), t('Cancel'));
}

/**
 * Page callback for AJAX enable/disable request on a codeblock.
 *
 * @param (int) $delta
 *   the primary key of the tracking code to enable/disable
 *
 * @return string
 *   a JSON object containing the status and label replacement
 */
function tracking_code_ajax_disable($delta) {
  $snippet = _tracking_code_read($delta);
  $status = ($snippet['status']) ? 0 : 1;

  db_update('tracking_code')
    ->fields(array('status' => $status))
    ->condition('tcid', $delta, '=')
    ->execute();

  $response = array(
    'status' => $status,
    'label' => ($status ? t('Disable') : t('Enable')),
  );

  print drupal_json_encode($response);
  exit();
}


/**
 * ### SUBMIT/VALIDATE FUNCTIONS ###
 */


/**
 * Validation function for tracking_code overview/weight table.
 */
function tracking_code_overview_form_validate($form, $form_state) {
  foreach ($form_state['values'] as $key => $value) {
    if (preg_match('/^weight_[0-9]+$/', $key)) {
      if (!is_numeric($value)) {
        form_set_error($key, t('Weight attribute must be a numeric value.'));
      }
    }
  }
}

/**
 * Submit function for tracking_code overview/weight table.
 */
function tracking_code_overview_form_submit($form, &$form_state) {
  foreach ($form_state['values'] as $key => $value) {
    if (preg_match('/^weight_[0-9]+$/', $key)) {
      $tcid = end(explode('weight_', $key));
      db_update('tracking_code')
        ->fields(array('weight' => $value))
        ->condition('tcid', $tcid, '=')
        ->execute();
    }
  }
  drupal_set_message(t('Tracking code weights have been updated.'));
}

/**
 * Submit function for tracking_code add form.
 */
function tracking_code_add_form_submit($form, &$form_state) {
  $tcid = db_insert('tracking_code')
    ->fields(array(
      'name' => $form_state['values']['tracking_code_name'],
      'code' => $form_state['values']['tracking_code_code'],
      'status' => 0,
      'weight' => 0,
      'visibility' => 1,
      'pages' => '',
      'content_types' => serialize(array()),
    ))
    ->execute();

  $form_state['redirect'] = 'admin/structure/tracking_code/' . $tcid . '/edit';
  drupal_set_message(t('Created new tracking code "%name."', array('%name' => $form_state['values']['tracking_code_name'])));
}

/**
 * Submit function for tracking_code edit form.
 */
function tracking_code_edit_form_submit($form, &$form_state) {
  db_update('tracking_code')
    ->fields(array(
      'name' => $form_state['values']['tracking_code_name'],
      'code' => $form_state['values']['tracking_code_code'],
      'region' => $form_state['values']['tracking_code_region'],
      'status' => $form_state['values']['tracking_code_status'],
      'visibility' => $form_state['values']['tracking_code_visibility'],
      'pages' => $form_state['values']['tracking_code_pages'],
      'content_types' => serialize($form_state['values']['tracking_code_content_types']),
    ))
    ->condition('tcid', $form_state['values']['delta'], '=')
    ->execute();

  $form_state['redirect'] = 'admin/structure/tracking_code';
  drupal_set_message(t('Updated tracking code "%name."', array('%name' => $form_state['values']['tracking_code_name'])));
}

/**
 * Submit function for tracking_code delete form.
 */
function tracking_code_delete_form_submit($form, &$form_state) {
  db_delete('tracking_code')
    ->condition('tcid', $form_state['values']['delta'])
    ->execute();

  $form_state['redirect'] = 'admin/structure/tracking_code';
  drupal_set_message(t('Deleted tracking code "%name."', array('%name' => $form_state['values']['snippet_name'])));
}
