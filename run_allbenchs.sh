benchmarks=('basicmath2020-03-0218:11.nvt' 'fft2020-01-191758.nvt' 'qsort2020-01-191912.nvt' 'typeset2020-01-191847.nvt' 'dijkstra2020-01-191729.nvt')

buffer=('16' '32' '64' '128' '256')

counter=('4' '8' '16' '32' '64')

ttm=('2048' '8192' '32768')

promote=('4' '5' '6' '7' '8' '9')

demote=('6' '5' '4' '3' '2' '1')


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
                       ./migrator input/$bench f $buff $c $t $pro $dem >> output/$bench.csv
                       #touch "output/$bench.csv"
                    done
                done
            done
            #echo $bench $buff $c
            #>> output/done.stat         
        done &
    done 
done 

