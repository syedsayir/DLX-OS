//The following notes focus on problem 7

/*Some notes:
-For this problem, SetupRootDir() below might help in setting up the root inode.

-You can copy as much code from lab4/flat as you want, including lab4/flat/apps/fdisk but make sure that you
make the necessary modifications.

-You can (optionally) do without a filename field in the inode since that information is stored in each directory entry.

-A sample inode might look as follows. You are free to differ.
struct inode {
tions mentioned in problem 7.
*/

//=================================================================

/*HELPER DEFINITIONS
//These are definitions which you MAY use. These are optional and have been randomly grouped together here.




//Various modes of file operations

#define R 		0x1	//Read mode
#define W 		0x2	//Write mode
#define RW 		0x3	//Read/write mode

//Various bits for file permissions

#define READ		0x4	//Read permission bit
#define WRITE		0x2	//Write permission bit
#define EXECUTE		0x1	//Execute permission bit

//Return values from various functions
#define INVALID_FILE		-1
#define INVALID_INODE		-1
#define FILE_EXISTS		-2
#define DOES_NOT_EXIST		-3
#define DIRECTORY_EXISTS	-4
#define NOT_A_DIRECTORY		-5
#define NOT_A_FILE		-6
#define INVALID_PATH		-7
#define PERMISSION_DENIED	-8
#define DIRECTORY_NOT_EMPTY	-9
*/

//=================================================================

/*HELPER FUNCTIONS
//These are helper functions which you MAY use. 
//They may not necessarily fit in directly with the given code. You can copy the logic and implement 
//your own exact ones.

struct filetab
{

  int Inuse;
  uint32 currpos;            // file's current position
  int Mode;                  // R/W/RW
  uint32 id;                 // owner id;
  INode *inode;              // pointer to the inode in memory

};
typedef struct filetab FD;


//--------------------------------------------------------------------------
extern INode inodes[MAXINODES]; //here, inode is the inode strcut
extern PCB *currentPCB;
static char *str="";
extern FD filetable[MAXOPENFILES];  //filetable is as defined above

//--------------------------------------------------------------------------
//	getInodeAddress
//
//	Given inode number, return its physical address, so that the inode
//	could be modified.
//--------------------------------------------------------------------------
INode *getInodeAddress(inode_t id)
{
  return (id>=0&&id<MAXINODES) ? &inodes[id] : NULL;
}

//-------------------------------------------------------------------------
//	bncmp
//
//	Works almost like strncmp() with a few differences. Returns 0 if both
//	the strings match to num bytes, or two their end (whichever occurs
//	first). Otherwise returns 1.
//-------------------------------------------------------------------------
int bncmp(char *s1, char *s2, int num)
{
  int i;

  for(i=0; i<num; i++)
  {
    if(s1[i] != s2[i])
      return 1;

    if(s1[i] =='0'&&s2[i]=='\0')
      return 0;
  }
  return 0;
}

//--------------------------------------------------------------------------
//	convertModeToInt
//	
//	Converts a mode string to integer. Mode "r" is 1, mode "w" is 2. The various 
//  combinations are obtained by adding these basic modes.
//--------------------------------------------------------------------------
int convertModeToInt(char *mode)
{
  int i;
  int md = 0;

  for(i=0; mode[i]!='\0'; i++)
  {
    switch(mode[i])
    {
      case 'r':
      case 'R':
        md |= R;
	break;
      case 'w':
      case 'W':
        md |= W;
	break;
      default:
        return 0;
    }
  }
  return md;
}

//---------------------------------------------------------------------------
//	checkPermission
//
//	Function to check whether the passed bits of a file are set. If the
//	specified permission is granted, it returns 1, otherwise it returns 0.
//---------------------------------------------------------------------------

int checkPermission(inode_t id, uint32 mode)
{
  INode *node;
  uint32 flmode;

  node = getInodeAddress(id);

  if(node==NULL)
  {
    return 0;				//inode not valid
  }
  
  flmode = node->Permission;

  if(node->ownerid==currentPCB->userid)
  {
    mode<<=3;
  }
  
  flmode &= mode;			//Mask all the bits other than the
  					//bits being tested
  flmode ^= mode;			//XOR with bits being tested. Result
  					//is 0 only if all the bits match 

  if(flmode)
  {
    return 0;				//permission denied
  }
  else
  {
    return 1;				//permission granted
  }
}  

//--------------------------------------------------------------------------
//	isDir
//
//	Checks whether the inode pointed to by id is a valid directory inode.
//	If that is the case, it returns 1. Otherwise it returns 0.
//--------------------------------------------------------------------------
int isDir(inode_t id)
{
  INode *node;
  uint32 flmode;

  node = getInodeAddress(id);

  if(node==NULL)
  {
    return 0;				//inode not valid
  }
  if(node->type==DIR)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}
  
//---------------------------------------------------------------------------
//	getOneName
//	
//	This function works much like strtok. The only difference is that the
//	field delimiter is fixed at '/', and you have to specify the
//	destination string where the token could be returned. The string dst
//	should have at least 31 bytes of space. This function also checks for
//	some error conditions such as extremely long filenames and successive
//	'/' characters. On success it returns the length of the string written
//	in dst (not to exceed 30). On error, it returns -1. If the string src
//	starts with a '/', the first '/' is ignored.
//
//	Example: Consider src = "/a/b/cd"
//		len = getOneName(dst, src); 
//			//returns dst = "a", len = 1;
//		len = getOneName(dst, NULL);
//			//returns dst = "b", len = 1;
//		len = getOneName(dst, NULL);
//			//returns dst = "cd", len = 2;
//		len = getOneName(dst, NULL);
//			//returns dst = "", len = 0;
//	
//	Note that the successive calls should be made with src = NULL. If src
//	is not NULL, the string passed is parsed from the beginnning.
//--------------------------------------------------------------------------
int 
getOneName(char *dst, char *src)
{
  int count = 0;

  if(src!=NULL)
  {
    if(*src=='/')
      src++;
    if(*src=='/')
      return -1;			//successive '/' not allowed
    str = src;	
  }
  for(;*str!='\0';str++)
  {
    dst[count] = *str;
    count++;
    if(*str=='/')
    {
      str++;
      count--;
      if(*str =='/')
      {
        return -1;			//successive '/' not allowed
      }
      break;
    }
    else
    {
      if(count==71) 
      {
       return -1;			//Filename too long
      }
    }
  }
  dst[count] = '\0';
  return count;
}

//--------------------------------------------------------------------------
//	MakeInodeFromPath
//
//	Given a path, and inode description, create that inode and return the
//	inode identifier. If a file corresponding to the path already exists, 
//	it returns FILE_EXISTS. This function is used both for creating a file 
//	and creating a directory. If the path provided is not valid, then this
//	function returns INVALID_PATH. If any directory along the path does
//	not have EXECUTE permission, or the parent directory of the leaf does
//	not have write permission, this function returns PERMISSION_DENIED.
//--------------------------------------------------------------------------
inode_t MakeInodeFromPath(char *path, int mode, int type)
{
  //This function is not mandatory, and you can skip writing it if you want.
  //But we strongly recommend writing this function as you can use this
  //function to create files as well as directories.
  
  //it may insert the inode number and file or directory name in global variables
}

//----------------------------------------------------------------------------
//	DeleteInodeGivenParent
//
//	Given the path, this function deletes the inode corresponding to that
//	path, and updates the directory entry in the parent directory.
//----------------------------------------------------------------------------
int DeleteInodeGivenParent(char *leafName, inode_t leaf, inode_t parent)
{
  //This function is not mandatory, and you can skip writing it if you want.
  //But we strongly recommend writing this function as you can use this
  //function to delete files as well as directories.
}

//***************************************************************


*/
