# bash completion for sf         -*- shell-script -*-

_sf()
{
    COMPREPLY=()
    local IFS=$'\n' 
    local cur=$2 prev=$3
    local -a options
    options=(
           --wrap
           --help
           --version
	   --theme
	   --settings-menu
    )
    if [[ $cur == -* ]]; then
        COMPREPLY=( $( compgen -W "${options[*]}" -- "$cur" ) )
    else
        COMPREPLY=( $( compgen -d -f -- "$cur") )
    fi

}

complete -o filenames -F _sf sf

