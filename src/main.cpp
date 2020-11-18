#include <stdio.h>
#include <windows.h>
#include <string.h>
#include <vector>
#define readbuf(buf) buf[3]+(buf[2]<<8)+(buf[1]<<16)+(buf[0]<<24)
long search(FILE *fp,const char fstr[],unsigned int size)
{
    unsigned int m = 0;
    unsigned char c;
    size--;
    while (fread(&c, 1, 1, fp) == 1)
    {
        if(c==fstr[m]){
            if(++m==size){
                return ftell(fp);
                break;
            }
        }else m=0;
    }
    return -1L;
}
int gcd(int a, int b){
    if(a%b == 0){
        return b;
    }else{
        return gcd(b, a%b);
    }
}
int gcd_vec(std::vector<int> &A) {
    unsigned int size = (int)A.size();
    unsigned int ret = A[0];
    for (unsigned int i = 1; i < size; i++) {
        ret = gcd(ret, A[i]);
    }
    return ret;
}
typedef struct note{
    unsigned char code[4]={0};//C4#
    unsigned int lenth;
}Note;


int main(int argc, char *argv[])
{
    FILE *fp;
    long int ftrack;
    unsigned int nc[4]={0};
    unsigned char buf[4];
    unsigned int variablelen = 0;
    unsigned int variablelenbuf[4] = {0};
    unsigned int track=0;
    if (argc == 1){
        return 0;
    }
    if (fopen_s(&fp, argv[1], "rb")){
        printf("ファイルオープンエラー %s", argv[1]);
        return 1;
    }


    std::vector<std::vector<Note>> Notes(4, std::vector<Note>(1));

    ftrack=search(fp,"MTrk",sizeof("MTrk")/sizeof(const char));
    fseek(fp,ftrack,SEEK_SET);
    fread(buf, 1, 4, fp);
    // len=readbuf(buf);
    while(1){
        if(fread(&variablelen, 1, 1, fp) == 1){
            if(variablelen>=0x80){
                fread(buf, 1, 4, fp);
                if(buf[0]>=0x80){
                    // printf("Error:too long");
                }
                variablelen=buf[0] + ((variablelen&0x7E)<<7) + (variablelen&1)*0x80;
            }else{
                fread(&buf[1], 1, 3, fp);
            }
            if(buf[1]==0xFF){//MetaEvent
                if(buf[2]==0x58&&buf[3]==0x04){
                    fread(buf, 1, 4, fp);
                }
            }
            track = buf[1]&0x0F;
            for(int i=0;i<4;i++)variablelenbuf[i]+=variablelen;
            if(buf[1]>>4==9){
                if(variablelenbuf[track]!=0){
                    if(Notes[track][nc[track]].code[0]!=0){
                        printf("Error:duplicate detected\nCh:%d\nNum:%d\nCode:%s",track+1,nc[track],Notes[track][nc[track]].code);
                        return 0;
                    }
                    sprintf((char*)Notes[track][nc[track]].code,"%c",'0');
                    Notes[track][nc[track]].lenth=variablelenbuf[track];
                    variablelenbuf[track]=0;
                    nc[track]++;
                    for(unsigned int i=0; i<nc[track]; i++){
                        Notes[track].resize(nc[track]+1);
                    }
                }
                if(Notes[track][nc[track]].code[0]!=0&&Notes[track][nc[track]].lenth==0){
                    // Notes[track][nc[track]].lenth=variablelen;
                    // nc[track]++;
                    // for(unsigned int i=0; i<nc[track]; i++){
                    //     Notes[track].resize(nc[track]+1);
                    // }
                    printf("Error:duplicate detected\nCh:%d\nNum:%d\nCode:%s",track+1,nc[track],Notes[track][nc[track]].code);
                    return 0;
                }

                char doremi[] = {'C','C','D','D','E','F','F','G','G','A','A','B'};
                switch(buf[2]%12){
                    case 0:
                    case 2:
                    case 4:
                    case 5:
                    case 7:
                    case 9:
                    case 11:
                        sprintf((char*)Notes[track][nc[track]].code,"%c%.1d%c",doremi[buf[2]%12],((buf[2]+3)/12)-1,0);
                        break;
                    default:
                    case 1:
                    case 3:
                    case 6:
                    case 8:
                    case 10:
                        sprintf((char*)Notes[track][nc[track]].code,"%c%.1d%c",doremi[buf[2]%12],((buf[2]+3)/12)-1,'S');
                        break;
                }
            }
            if(buf[1]>>4==8){
                Notes[track][nc[track]].lenth=variablelenbuf[track];
                    variablelenbuf[track]=0;
                // printf("{%s,%d}",Notes[track][nc[track]].code,Notes[track][nc[track]].lenth);
                nc[track]++;
                for(unsigned int i=0; i<nc[track]; i++){
                    Notes[track].resize(nc[track]+1);
                }
            }
            variablelen=0;
        }else break;
    }
    unsigned int gcd;
        std::vector<std::vector<int>> a(5, std::vector<int>(1));
            a[4].resize(4);
    for(int i = 0;i<4;i++){
        if(nc[i]){
            a[i].resize(nc[i]);
            for(unsigned int g = 0;g<nc[i];g++){
                a[i][g]=Notes[i][g].lenth;
            }
            // printf("%d",gcd_vec(a[i]));
        }else a[i][0]=480;
        a[4][i]=gcd_vec(a[i]);
    }
    gcd=gcd_vec(a[4]);
    printf("約数:%d\nTotalLenth:%d\n",gcd,nc[0]+nc[1]+nc[2]+nc[3]);

    for(int i = 0;i<4;i++){
        if(!nc[i])continue;
        printf("const PROGMEM unsigned char track%d[][2]={",i+1);
        for(unsigned int g = 0;g<nc[i];g++){
            int buf=Notes[i][g].lenth/gcd;
            if(Notes[i][g].lenth/gcd>255){
                while(buf>255){
                    printf("{%s,%d},",Notes[i][g].code,255);
                    buf-=255;
                }
                printf("{%s,%d}",Notes[i][g].code,Notes[i][g].lenth/gcd%255);
                if(g+1<nc[i])printf(",");
            }else if(g!=nc[i]){
                if(!strcmp((const char*)Notes[i][g].code,(const char*)Notes[i][g+1].code)){
                    printf("{%s,%d},{0,1}",Notes[i][g].code,Notes[i][g].lenth/gcd-1);
                }else{
                    printf("{%s,%d}",Notes[i][g].code,Notes[i][g].lenth/gcd);
                }
                if(g+1<nc[i])printf(",");
            }
        }
        printf("};\n");
    }
    fclose(fp);
    return 0;
}