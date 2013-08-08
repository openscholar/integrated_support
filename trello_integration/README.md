Trello Integration
==================

What does it do?
----------------

1. Provides trello crud
2. Roadmap < trello
3. Webhooks -> other services
  1. Desk
  2. Github
  3. GetSatisfaction
  
Webhooks
--------

**What changes get pushed?**

1. List.  Dev -> QA means the ticket is progressing.
  Complete, deployed should trigger messages to GS and Desk
  Should other changes trigger label changes in GH?  Or the other way around?

What else is there?  Text, description, labels, members, checklist, due date, file attachment...

Things that trigger a webhook:
 * Edit description
 * Add comment
 * Change labels
 * Assign member
 * Add checklist
 * Move card between lists (includes listAfter and listBefore)
 
Things that don't:
  * Edit comment (requested)
  * Delete comment (requested)

**How do issues get added**

1. Getsatisfaction: 
  * cron
    status = active and not in list.
    or status = pending.  once out of inbox, make it active.  or if deleted, close it.
2. GitHub: cron or webhook?
  * cron: 
    match a label
  * webhook: at modify w/ label.
  
  (shouldn't everything in github be in the roadmap?  no, then we'd have redundant entries)
3. Desk: 
  * action?
    When someone creates a bug report?  
    or
    When someone sends to trello?
  * cron
    
**If issues are added automagically, how do I link an existing item to a specific card?**

1. You can't.
2. Send to trello could have create new or add to, which does text search.
  * GS has mergable topics.  You'd merge a topic into the topic with the link  
  * GH wouldn't do this, I hope.
3. By title.  Cloned issues are the most likely to be redundant.  Check if the title has a match before trello-ing it and then update or create.
    
**What external changes will update a trello card?**

1. None.  Trello is the master.  There is only Trello.

Even GH status changes?

2. Yes

Even GH milestone changes?

3. Maybe.


**Remaining questions**

How would I get the date of the next release if we only mark things as "next"?  Is next good enough?

Should cards have link status comments?  ie "This card controls desk/321".  If yes, could removing them remove the link?
  Removing comment -> disabling webhook won't work.  Card doesn't send a webhook when comment is edited or deleted.  I tried adding a webhook to the comment, and that went through, but it never fired.

Trello cards have a due date.  Use it?  Or is it redundant/mutually exclusive with the "next" label of the list.

When I fetch a card can I see its webhooks?  Does this help in any way?

Does this obviate sending from one service to another?  If not, what ends up in trello?

