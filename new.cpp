#include<bits/stdc++.h>
#include "jsoncpp/json.h"
using namespace std;
mt19937 rnd(1);
int Rnd(int l,int r) {return rnd()%(r-l+1)+l;}
const int n=11;
int dx[]={-1,-1,0,0,1,1},dy[]={0,1,-1,1,-1,0};
inline int gid(int x,int y) {return x*n+y;}
inline void revid(int id,int &x,int &y) {x=id/n,y=id%n;}
int mineColor,visitedCount=1;
const double exploration=0.4;
vector<pair<int,int>> allxy;
const int STATECOUNT=10000005;
class State
{
public:
	int board[11][11];
	// 0 : empty, 1 red, -1 blue
	int fa[125];
	// 121 ~ 124 : UDLR
	int current;
	int find(int u) {return fa[u]==u?u:fa[u]=find(fa[u]);}
	State()
	{
		memset(board,0,sizeof(board));
		for(int i=0;i<125;i++) fa[i]=i;
		current=1;
	}
	int color(int x,int y) {return board[x][y];}
	void place(int x,int y)
	{
		assert(board[x][y]==0);
		board[x][y]=current;
		for(int i=0;i<6;i++)
		{
			int tx=x+dx[i],ty=y+dy[i];
			if(tx>=0&&tx<n&&ty>=0&&ty<n)
			{
				if(board[x][y]==board[tx][ty]) fa[find(gid(x,y))]=find(gid(tx,ty));
			}
			else
			{
				if(current==1)
				{
					if(tx==-1) fa[find(gid(x,y))]=find(121);
					if(tx==n) fa[find(gid(x,y))]=find(122);
				}
				else
				{
					if(ty==-1) fa[find(gid(x,y))]=find(123);
					if(ty==n) fa[find(gid(x,y))]=find(124);
				}
			}
		}
		current*=-1;
	}
	void chkbridge()
	{
		for(int x=0;x<n;x++) for(int y=0;y<n;y++) if(board[x][y]!=0)
		{
			if(x+1<n&&y-2>0&&board[x][y]==board[x+1][y-2])
			{
				if(color(x+1,y-1)==0&&color(x,y-1)==0)
				{
					if(rnd()&1) place(x+1,y-1),place(x,y-1);
					else place(x,y-1),place(x+1,y-1);
				}
			}
			if(x+1<n&&y+1<n&&color(x,y)==color(x+1,y+1))
			{
				if(color(x,y+1)==0&&color(x+1,y)==0)
				{
					if(rnd()&1) place(x,y+1),place(x+1,y);
					else place(x+1,y),place(x,y+1);
				}
			}
			if(x+2<n&&y-1>0&&color(x,y)==color(x+2,y-1))
			{
				if(color(x+1,y-1)==0&&color(x+1,y)==0)
				{
					if(rnd()&1) place(x+1,y-1),place(x+1,y);
					else place(x+1,y),place(x+1,y-1);
				}
			}
			if(color(x,y)==1&&x==1&&y<n-1)
			{
				if(color(x-1,y)==0&&color(x-1,y+1)==0)
				{
					if(rnd()&1) place(x-1,y),place(x-1,y+1);
					else place(x-1,y+1),place(x-1,y);
				}
			}
			if(color(x,y)==1&&x==n-2&&y>0)
			{
				if(color(x+1,y-1)==0&&color(x+1,y)==0)
				{
					if(rnd()&1) place(x+1,y),place(x+1,y-1);
					else place(x+1,y-1),place(x+1,y);
				}
			}
			if(color(x,y)==-1&&x<n-1&&y==1)
			{
				if(color(x,y-1)==0&&color(x+1,y-1)==0)
				{
					if(rnd()&1) place(x,y-1),place(x+1,y-1);
					else place(x+1,y-1),place(x,y-1);
				}
			}
			if(color(x,y)==-1&&x>0&&y==n-2)
			{
				if(color(x,y+1)==0&&color(x-1,y+1)==0)
				{
					if(rnd()&1) place(x,y+1),place(x-1,y+1);
					else place(x-1,y+1),place(x,y+1);
				}
			}
		}
	}
	int redWin() {return find(121)==find(122);}
	int blueWin() {return find(123)==find(124);}
	int isend() {return redWin()||blueWin();}
	void print()
	{
		printf("current : %c\n",current==1?'O':'X');
		for(int i=0;i<n;i++)
		{
			for(int j=0;j<n;j++) printf("%c","X.O"[board[i][j]+1]);
			printf("\n");
		}
	}
}nowstate,input;
int dist[11][11],qx[121],qy[121],id[121];
int LIMDist;
struct MTCS
{
	// unordered_map<int,int> child[STATECOUNT];
	int val[STATECOUNT],nxt[STATECOUNT],fir[STATECOUNT];
	int vistime[STATECOUNT],wintime[STATECOUNT];
	int curid,record;
	pair<int,int> getnextmove()
	{
		if(!fir[curid])
		{
			memset(dist,-1,sizeof(dist));
			int ql=0,qr=-1;
			for(int i=0;i<n;i++) for(int j=0;j<n;j++) if(nowstate.color(i,j)!=0) dist[i][j]=0,qr++,qx[qr]=i,qy[qr]=j;
			int cnt=0;
			while(ql<=qr)
			{
				int x=qx[ql],y=qy[ql]; ql++;
				if(dist[x][y]==LIMDist) break;
				for(int i=0;i<6;i++)
				{
					int tx=x+dx[i],ty=y+dy[i];
					if(tx>=0&&tx<n&&ty>=0&&ty<n&&dist[tx][ty]==-1)
					{
						dist[tx][ty]=dist[x][y]+1;
						qr++,qx[qr]=tx,qy[qr]=ty;
						id[++cnt]=gid(tx,ty);
					}
				}
			}
			shuffle(id+1,id+cnt+1,rnd);
			for(int j=1;j<=cnt;j++)
			{
				int nw=++visitedCount;
				val[nw]=id[j],nxt[nw]=fir[curid];
				fir[curid]=nw;
			}
			return {val[fir[curid]],-fir[curid]};
		/*
			shuffle(allxy.begin(),allxy.end(),rnd);
			for(auto [x,y]:allxy)
			{
				if(nowstate.color(x,y)!=0) continue;
				int nw=++visitedCount;
				val[nw]=gid(x,y),nxt[nw]=fir[curid];
				fir[curid]=nw;
			}
		*/
		}
		int mxid=0,mxw=0; double mxv=-1e20;
		for(int i=fir[curid];i!=0;i=nxt[i])
		{
			if(!vistime[i]) return {val[i],i};
			double curvalue=(double)wintime[i]/vistime[i]
							+exploration*sqrt(log(vistime[curid])/vistime[i]);
			if(curvalue>mxv) mxv=curvalue,mxid=val[i],mxw=i;
		}
		return {mxid,mxw};
	}
	void randomplay()
	{
		shuffle(allxy.begin(),allxy.end(),rnd);
		for(auto [x,y]:allxy)
		{
			if(nowstate.color(x,y)==0)
			{
				nowstate.place(x,y);
				if(x-2>=0&&y+1<n&&nowstate.color(x,y)==nowstate.color(x-2,y+1))
				{
					if(nowstate.color(x-1,y)==0&&nowstate.color(x-1,y+1)==0)
					{
						if(rnd()&1) nowstate.place(x-1,y),nowstate.place(x-1,y+1);
						else nowstate.place(x-1,y+1),nowstate.place(x-1,y);
					}
				}
				if(x-1>=0&&y-1>=0&&nowstate.color(x,y)==nowstate.color(x-1,y-1))
				{
					if(nowstate.color(x-1,y)==0&&nowstate.color(x,y-1)==0)
					{
						if(rnd()&1) nowstate.place(x-1,y),nowstate.place(x,y-1);
						else nowstate.place(x,y-1),nowstate.place(x-1,y);
					}
				}
				if(x-1>=0&&y+2<n&&nowstate.color(x,y)==nowstate.color(x-1,y+2))
				{
					if(nowstate.color(x-1,y+1)==0&&nowstate.color(x,y+1)==0)
					{
						if(rnd()&1) nowstate.place(x-1,y+1),nowstate.place(x,y+1);
						else nowstate.place(x,y+1),nowstate.place(x-1,y+1);
					}
				}
				if(x+1<n&&y-2>0&&nowstate.color(x,y)==nowstate.color(x+1,y-2))
				{
					if(nowstate.color(x+1,y-1)==0&&nowstate.color(x,y-1)==0)
					{
						if(rnd()&1) nowstate.place(x+1,y-1),nowstate.place(x,y-1);
						else nowstate.place(x,y-1),nowstate.place(x+1,y-1);
					}
				}
				if(x+1<n&&y+1<n&&nowstate.color(x,y)==nowstate.color(x+1,y+1))
				{
					if(nowstate.color(x,y+1)==0&&nowstate.color(x+1,y)==0)
					{
						if(rnd()&1) nowstate.place(x,y+1),nowstate.place(x+1,y);
						else nowstate.place(x+1,y),nowstate.place(x,y+1);
					}
				}
				if(x+2<n&&y-1>0&&nowstate.color(x,y)==nowstate.color(x+2,y-1))
				{
					if(nowstate.color(x+1,y-1)==0&&nowstate.color(x+1,y)==0)
					{
						if(rnd()&1) nowstate.place(x+1,y-1),nowstate.place(x+1,y);
						else nowstate.place(x+1,y),nowstate.place(x+1,y-1);
					}
				}
				if(nowstate.color(x,y)==1&&x==1&&y<n-1)
				{
					if(nowstate.color(x-1,y)==0&&nowstate.color(x-1,y+1)==0)
					{
						if(rnd()&1) nowstate.place(x-1,y),nowstate.place(x-1,y+1);
						else nowstate.place(x-1,y+1),nowstate.place(x-1,y);
					}
				}
				if(nowstate.color(x,y)==1&&x==n-2&&y>0)
				{
					if(nowstate.color(x+1,y-1)==0&&nowstate.color(x+1,y)==0)
					{
						if(rnd()&1) nowstate.place(x+1,y),nowstate.place(x+1,y-1);
						else nowstate.place(x+1,y-1),nowstate.place(x+1,y);
					}
				}
				if(nowstate.color(x,y)==-1&&x<n-1&&y==1)
				{
					if(nowstate.color(x,y-1)==0&&nowstate.color(x+1,y-1)==0)
					{
						if(rnd()&1) nowstate.place(x,y-1),nowstate.place(x+1,y-1);
						else nowstate.place(x+1,y-1),nowstate.place(x,y-1);
					}
				}
				if(nowstate.color(x,y)==-1&&x>0&&y==n-2)
				{
					if(nowstate.color(x,y+1)==0&&nowstate.color(x-1,y+1)==0)
					{
						if(rnd()&1) nowstate.place(x,y+1),nowstate.place(x-1,y+1);
						else nowstate.place(x-1,y+1),nowstate.place(x,y+1);
					}
				}
			}
			if(nowstate.isend()) return ;
		}
	}
	void runMTCS()
	{
		nowstate=input;
		vector<int> vis(1,1);
		curid=1,record=1;
//		printf("* \n");
		while(!nowstate.isend())
		{
//			printf("%d %d\n",record,curid);
			auto [option,newid]=getnextmove();
			nowstate.place(option/n,option%n);
			if(newid<0) break;
			vis.push_back(newid);
			curid=newid;
		}
		State cur=nowstate;
		cur.chkbridge();
		for(int i=0;i<3;i++)
		{
			nowstate=cur;
			randomplay();
			int winner=nowstate.redWin()?1:-1;
			int cop=input.current;
			for(int ids:vis)
			{
				vistime[ids]++;
				if(cop!=winner) wintime[ids]++;
				cop=-cop;
			}
		}
	}
	int getoption()
	{
		int maxwin=-1,maxid=0;
		for(int i=fir[1];i!=0;i=nxt[i])
		{
			if(wintime[i]>maxwin) maxwin=wintime[i],maxid=val[i];
		}
		#ifdef wasa855
			vector<pair<int,int>> res;
			for(int i=fir[1];i!=0;i=nxt[i])
			{
				res.emplace_back(val[i],wintime[i]);
			}
			sort(res.begin(),res.end());
			for(auto [x,y]:res) printf("%d %d %d\n",x/n,x%n,y);
		#endif
		return maxid;
	}
}mtcs;
int starttime,cnt=0;
int getans()
{
	while((double)(clock()-starttime)/CLOCKS_PER_SEC<0.9&&visitedCount<=STATECOUNT-130) mtcs.runMTCS(),cnt++;
	#ifdef wasa855
		cout<<cnt<<endl;
	#endif
	return mtcs.getoption();
}
int actions_3thplay[]={
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,80,40,45,80,48,50,45,-1,
-1,35,73,34,40,40,73,73,45,-1,-1,
-1,73,80,80,50,80,61,73,80,45,45,
-1,80,80,45,45,50,42,80,80,50,58,
40,70,45,59,45,80,80,80,80,72,80,
84,85,51,45,45,70,80,80,80,51,80,
59,80,80,51,45,80,70,70,80,72,60,
50,59,45,69,80,80,80,70,70,80,72,
42,71,100,80,80,70,96,70,70,80,40,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
};
int actions_4thplay[]={
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
40,40,-1,40,40,40,28,40,95,51,62,
71,40,40,40,71,40,96,47,83,62,95,
95,36,95,95,95,50,95,83,83,40,20,
40,71,71,71,71,71,71,62,29,40,40,
92,95,40,71,71,71,40,40,40,96,96,
40,40,59,71,81,40,40,84,40,96,40,
40,93,90,-1,70,71,50,50,40,107,40,
47,40,79,71,71,40,40,84,50,50,40,
40,40,40,40,40,50,50,40,40,40,40,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
};
Json::Value get_next_action(bool forced_flag){
	int action;
	if (forced_flag){
        action = 1*11+2;
    }
    else
    {
    	int cnt=0;
    	for(int i=0;i<n;i++) for(int j=0;j<n;j++) if(input.color(i,j)!=0) cnt++;
    	if(cnt<=6) LIMDist=3;
    	else LIMDist=2;
    	if(cnt==1) action=gid(7,3);
    	else if(cnt==2)
		{
			int pid=0;
			for(int i=0;i<n;i++) for(int j=0;j<n;j++)
			{
				if(i==1&&j==2) continue;
				if(input.color(i,j)!=0) pid=gid(i,j);
			}
			if(actions_3thplay[pid]==-1) LIMDist=20,action=getans();
			else action=actions_3thplay[pid];
		}
		else if(cnt==3)
		{
			int pid=0;
			for(int i=0;i<n;i++) for(int j=0;j<n;j++)
			{
				if(i==1&&j==2) continue;
				if(i==7&&j==3) continue;
				if(input.color(i,j)!=0) pid=gid(i,j);
			}
			if(actions_3thplay[pid]==-1) LIMDist=20,action=getans();
			else action=actions_4thplay[pid];
		}
		else action=getans();
    }

	Json::Value action_json;
	action_json["x"] = action / n;
	action_json["y"] = action % n;
	return action_json;
}
signed main()
{
#ifdef wasa855
	starttime=clock();
	freopen("a.in","r",stdin);
	freopen("a.out","w",stdout);
#endif
	for(int i=0;i<n;i++) for(int j=0;j<n;j++) allxy.emplace_back(i,j);
	string str;
	getline(cin, str);
	Json::Reader reader;
	Json::Value Input;
	reader.parse(str, Input);
	
	int turn_id = Input["responses"].size();
    int x, y;
    bool forced_flag;
	for (int i = 0; i < turn_id; i++) {
		x = Input["requests"][i]["x"].asInt();
        y = Input["requests"][i]["y"].asInt();
        if(i==0)
        {
        	if(x>=0&&y>=0) mineColor=-1;
        	else mineColor=1;
        }
        if (x >= 0 and y >= 0){
            input.place(x,y); 
        }
        x = Input["responses"][i]["x"].asInt();
        y = Input["responses"][i]["y"].asInt();
        input.place(x,y);
	}
    x = Input["requests"][turn_id]["x"].asInt();
    y = Input["requests"][turn_id]["y"].asInt();

    if (x >= 0 and y >= 0){
    	input.place(x,y);
        forced_flag = false;
    } else if (Input["requests"][(int)0].isMember("forced_x")){
        forced_flag = true;
    } else {
        forced_flag = false;
    }
		
	Json::Value result;
	
    result["response"] = get_next_action(forced_flag);

	Json::FastWriter writer;
	
    cout << writer.write(result) << endl;
	return 0;
} 