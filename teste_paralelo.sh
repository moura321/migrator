benchmarks=('basicmath2020-01-191653.nvt' 'dijkstra2020-01-191729.nvt' 'fft2020-01-191758.nvt' 'qsort2020-01-191912.nvt' 'typeset2020-01-191847.nvt')

#allread tested:   buffer=('16' '32' '64' '128')
buffer=('256')

counter=('4' '8' '16' '32' '64')

ttm=('2048' '8192' '32768')

promote=('4' '5' '6' '7' '8' '9')

demote=('6' '5' '4' '3' '2' '1')

j=0

for bench in "${benchmarks[@]}"
do 
    for buff in "${buffer[@]}"
    do 
        for c in "${counter[@]}"
        do
            for t in "${ttm[@]}"
            do
                for pro in "${promote[@]}"
                do
                    for dem in "${demote[@]}"
                    do
                       #./l input/$bench $buff $c $t $pro $dem >> teste/$bench.csv
                       touch "teste/$bench.csv"
                       echo dentro
                    done
                done
            done        
        done
    done &
done

