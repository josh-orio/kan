# KAN - The only Kanban app with a TUI

KAN is a terminal based project managment tool that lives entirely in your repository!

## File Structure

KAN data is supposed to be part of the repo, not an external site like Jira.

```
.kan
|
|- boards/
   |
   |- columns-{todo, in progress, ready, done}/ 
      |
      |- ticket-1.bson
      |- ticket-2.bson
```