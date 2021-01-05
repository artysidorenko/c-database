# Build a SQLITE clone

Follows the tutorial - <https://cstack.github.io/db_tutorial/> - up to part 13, at which point the tutorial is discontinued, leaving me to improvise ¯\_(ツ)_/¯

## Testing

`sudo gem install rspec`
`rspec ./spec/database.rb`

## Developing

Compiling the codebase into an executable file:

`gcc-10 main.c -o dist/database`
