# bash completion for show         -*- shell-script -*-

_show()
{
    COMPREPLY=()
    local IFS=$'\n' 
    local cur=$2 prev=$3
    local -a options
    options=(
           -@
           --all
           --author
	   --block-size
           --ignore-backups
           --color
           --directroy
           -f
           --full-time
           -g
           --no-group
           --human-readable
	   --inode
           --si
	   --numeric-uid-gid
           --reverse
           --size
           -S
           --time-style
           -t
           -U
           --context
           -1
           --help
           --version
	   --directories-only
	   --theme
	   --no-danger
	   --marked
	   --no-sf
	   --show-on-enter
	   --running
	   --settings-menu
	   --edit-themes
	   --skip-to-first
	   --enable-mouse
    )
    if [[ $cur == -* ]]; then
        COMPREPLY=( $( compgen -W "${options[*]}" -- "$cur" ) )
    else
        COMPREPLY=( $( compgen -d -- "$cur") )
    fi

}

complete -o dirnames -F _show show

