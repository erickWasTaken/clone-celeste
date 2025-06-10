wrules=".hyprrules"

if [ -f "$wrules" ]; then
    while IFS= read -r line; do
        [[ -z "$line" || "$line" == \#* ]] && continue
        hyprctl keyword "$line"
    done < "$wrules"
fi

./app

