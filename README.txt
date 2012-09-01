Tracking Code Module


Description
===========
  A module that allows admins to create tracking code snippets and control their
  visibility similar to the core Drupal block module. It can be used to inject
  arbitrary code into the <head>, and at the beginning and end of the <body>
  tag, the most common placement for tracking code.

  This module allows you to name your various tracking code snippets and 
  organize them by weight and region. Configuration of a single snippet provides
  visibility settings similar to the core Drupal block module: white/black list
  by Drupal path and content type.

  Currently there are three possible regions to inject your tracking code:
   * Inside <HEAD>  - Code added using drupal_add_html_head()
   * After <BODY> - Code added to $page['page_top'] in hook_page_alter()
   * Before </BODY> - Code added to $page['content_bottom'] in hook_page_alter()

  You can also use drag & drop functionality on the tracking code overview page 
  to change the relative weights of your snippets to make sure they execute in
  the correct order within a given region.


Configuration
=============
  * Only roles with the "administer tracking code" permission can access the
    administrative page and configure this module.
  * To configure the tracking code module, navigate to:
    Admin > Structure > Tracking Code Snippets (/admin/structure/tracking_code)
  * The "List" tab will display all the tracking code snippets you've created.
    It provides a drag & drop weight table and links for enabling/disabling a
    snippet, editing a snippet, or deleting a snippet.
  * The "Add Tracking Code" tab will display a form for creating a new snippet.
  * Once you've added a snippet, you are presented with further configuration
    options such as changing the active/inactive status, white/black listing
    page visibility, and choosing content types to restrict visiblity to. These
    options were designed to look/feel/behave similar to Drupal's core block
    module.


Acknowledgements
================
  Initial development of the tracking_code module was sponsored by Allora Media
  (http://www.alloramedia.com). 


Author
======
  Joe Pitassi (http://drupal.org/user/382188, IRC: ColonelForbinX)