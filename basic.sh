benchmarks=('basicmath2020-01-191653.nvt')
buffer=('16' '32' '64' '128')
counter=('4' '8' '16' '32' '64')
ttm=('2048' '8192' '32768')
promote=('4' '5' '6' '7' '8' '9')
demote=('6' '5' '4' '3' '2' '1')

j=0

for dem in "${demote[@]}"
do
    for buff in "${buffer[@]}"
    do
        for c in "${counter[@]}"
        do
            date
            let "j=j+1"
            echo "$j/120"      
            for t in "${ttm[@]}"
            do
                for pro in "${promote[@]}"
                do
                    for bench in "${benchmarks[@]}"
                    do
                       ./l input/$bench $buff $c $t $pro $dem >> output/$bench.csv
                       #touch "output/$bench.csv"
                    done
                done
            done        
        done
    done
done

