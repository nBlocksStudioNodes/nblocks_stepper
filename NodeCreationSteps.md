# New nBlocksSDtudio Node creation steps

* Server
   * Login to nBlocks Studio Server
   * Define the new Node
   * Add the new Node to 'my Nodes'
*  Download the file `user_data.json` from the server, calling locally the Translator [todo: a new script to download without translating]
*  Create the local `node.json`
   * Copying the Node defintion from `user_data.json` 
   * If there is no Internet connetion
      * Create the Node definition from scratch
      * Or copy-modify the local Node `node.json` from another Local Node, 
*  Create local git repo
*  Create remote gitHub repo
*  sync the local and remote repos
*  sceleton code creation  [todo: Create guidlines and references to existing Nodes]
*  competed code
*  Create a single Design in ascii DSL or in Schematic and translate the Design to cpp project
   * Iterate untill no errors
   * Alternatively create a single Node cpp project manually [todo: create a template directory]
*  Single node compilation 
   * Iterate untill no errors and compilation is successfull
*  Create testing Design with one or more Nodes connected to the new Node
*  Translate the design
   * Iterate untill no errors
*  Compile the Project
   * Iterate untill no compilation errors
* Complete the Node "readme.md"
* Push the final repo version to the remote repo
* Update the Server if needed


Push to the remote repo as often as possible 
* If your work is interrupted, it can be continued from yourself or another collaborator.
* Other Node Developers are aware of what is in progress
* Dont wait untill the code is perfect to commit and push
* Act as if your PC can be broken at any time. Make your work accessible

